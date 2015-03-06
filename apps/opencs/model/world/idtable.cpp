#include "idtable.hpp"
#include <QDebug>

#include <cassert>
#include <iostream>
#include "nestedtablewrapper.hpp"

#include "collectionbase.hpp"
#include "columnbase.hpp"

CSMWorld::IdTable::IdTable (CollectionBase *idCollection, unsigned int features)
: IdTableBase (features), mIdCollection (idCollection)
{}

CSMWorld::IdTable::~IdTable()
{}

int CSMWorld::IdTable::rowCount (const QModelIndex & parent) const
{
    if (hasChildren(parent))
    {
        return dynamic_cast<NestedCollection*>(mIdCollection)->getNestedRowsCount(parent.row(), parent.column());
    }

    return mIdCollection->getSize();
}

int CSMWorld::IdTable::columnCount (const QModelIndex & parent) const
{
    if (hasChildren(parent))
    {
        return dynamic_cast<NestedCollection*>(mIdCollection)->getNestedColumnsCount(parent.row(), parent.column());
    }

    return mIdCollection->getColumns();
}

QVariant CSMWorld::IdTable::data  (const QModelIndex & index, int role) const
{
    if ((role!=Qt::DisplayRole && role!=Qt::EditRole) || index.row() < 0 || index.column() < 0)
        return QVariant();

    if (role==Qt::EditRole && !mIdCollection->getColumn (index.column()).isEditable())
        return QVariant();

    if (index.internalId() != 0)
    {
        std::pair<int, int> parentAdress(unfoldIndexAdress(index.internalId()));
        return dynamic_cast<NestedCollection*>(mIdCollection)->getNestedData(parentAdress.first,
                                                                             parentAdress.second,
                                                                             index.row(),
                                                                             index.column());
    } else {
        return mIdCollection->getData (index.row(), index.column());
    }
}

QVariant CSMWorld::IdTable::headerData (int section,
                                        Qt::Orientation orientation,
                                        int role) const
{
    if (orientation==Qt::Vertical)
        return QVariant();

    if (role==Qt::DisplayRole)
        return tr (mIdCollection->getColumn (section).getTitle().c_str());

    if (role==ColumnBase::Role_Flags)
        return mIdCollection->getColumn (section).mFlags;

    if (role==ColumnBase::Role_Display)
    {
        return mIdCollection->getColumn (section).mDisplayType;
    }

    return QVariant();
}

QVariant CSMWorld::IdTable::nestedHeaderData(int section, int subSection, Qt::Orientation orientation, int role) const
{
    const NestColumn& parentColumn = dynamic_cast<const NestColumn&>(mIdCollection->getColumn(section));

    if (orientation==Qt::Vertical)
        return QVariant();

    if (role==Qt::DisplayRole)
        return tr(parentColumn.nestedColumn(subSection).getTitle().c_str());

    if (role==ColumnBase::Role_Flags)
        return mIdCollection->getColumn (section).mFlags;

    if (role==ColumnBase::Role_Display)
        return parentColumn.nestedColumn(subSection).mDisplayType;

    return QVariant();
}

bool CSMWorld::IdTable::setData (const QModelIndex &index, const QVariant &value, int role)
{
    if (index.internalId() != 0)
    {
        if (mIdCollection->getColumn(parent(index).column()).isEditable() && role==Qt::EditRole)
        {
            const std::pair<int, int>& parentAdress(unfoldIndexAdress(index.internalId()));

            dynamic_cast<NestedCollection*>(mIdCollection)->setNestedData(parentAdress.first, parentAdress.second, value, index.row(), index.column());

            emit dataChanged (CSMWorld::IdTable::index (parentAdress.first, 0),
                              CSMWorld::IdTable::index (parentAdress.second, mIdCollection->getColumns()-1));

            return true;
        } else
        {
            return false;
        }
    }

    if (mIdCollection->getColumn (index.column()).isEditable() && role==Qt::EditRole)
    {
        mIdCollection->setData (index.row(), index.column(), value);

        emit dataChanged (CSMWorld::IdTable::index (index.row(), 0),
                          CSMWorld::IdTable::index (index.row(), mIdCollection->getColumns()-1));

        return true;
    }

    return false;
}

Qt::ItemFlags CSMWorld::IdTable::flags (const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (mIdCollection->getColumn (index.column()).isUserEditable())
        flags |= Qt::ItemIsEditable;

    return flags;
}

bool CSMWorld::IdTable::removeRows (int row, int count, const QModelIndex& parent)
{
    beginRemoveRows (parent, row, row+count-1);

    if (parent.isValid())
    {
        for (int i = 0; i < count; ++i)
        {
            dynamic_cast<NestedCollection*>(mIdCollection)->removeNestedRows(parent.row(), parent.column(), row+i);
        }
    } else
    {

        beginRemoveRows (parent, row, row+count-1);

        mIdCollection->removeRows (row, count);
    }

    endRemoveRows();

    emit dataChanged (CSMWorld::IdTable::index (parent.row(), 0),
                      CSMWorld::IdTable::index (parent.row(), mIdCollection->getColumns()-1));

    return true;
}

void CSMWorld::IdTable::addNestedRow(const QModelIndex& parent, int position)
{
    if (!hasChildren(parent))
    {
        throw std::logic_error("Tried to set nested table, but index has no children");
    }

    int row = parent.row();

    beginInsertRows(parent, position, position);
    dynamic_cast<NestedCollection*>(mIdCollection)->addNestedRow(row, parent.column(), position);

    endInsertRows();

    emit dataChanged (CSMWorld::IdTable::index (row, 0),
                      CSMWorld::IdTable::index (row, mIdCollection->getColumns()-1));
}

QModelIndex CSMWorld::IdTable::index (int row, int column, const QModelIndex& parent) const
{
    unsigned int encodedId = 0;
    if (parent.isValid())
    {
        encodedId = this->foldIndexAdress(parent);
    }

    if (row<0 || row>=mIdCollection->getSize())
        return QModelIndex();

    if (column<0 || column>=mIdCollection->getColumns())
        return QModelIndex();

    return createIndex(row, column, encodedId);
}

QModelIndex CSMWorld::IdTable::parent (const QModelIndex& index) const
{
    if (index.internalId() == 0) //0 is used for indexs with invalid parent (top level data)
    {
        return QModelIndex();
    }

    unsigned int id = index.internalId();
    const std::pair<int, int>& adress(unfoldIndexAdress(id));

    if (adress.first >= this->rowCount() || adress.second >= this->columnCount())
    {
        throw "Parent index is not present in the model";
    }
    return createIndex(adress.first, adress.second);
}

void CSMWorld::IdTable::addRecord (const std::string& id, UniversalId::Type type)
{
    int index = mIdCollection->getAppendIndex (id, type);

    beginInsertRows (QModelIndex(), index, index);

    mIdCollection->appendBlankRecord (id, type);

    endInsertRows();
}

void CSMWorld::IdTable::cloneRecord(const std::string& origin,
                                    const std::string& destination,
                                    CSMWorld::UniversalId::Type type)
{
    int index = mIdCollection->getAppendIndex (destination);

    beginInsertRows (QModelIndex(), index, index);
    mIdCollection->cloneRecord(origin, destination, type);
    endInsertRows();
}

///This method can return only indexes to the top level table cells
QModelIndex CSMWorld::IdTable::getModelIndex (const std::string& id, int column) const
{
    return index(mIdCollection->getIndex (id), column);
}

void CSMWorld::IdTable::setRecord (const std::string& id, const RecordBase& record)
{
    int index = mIdCollection->searchId (id);

    if (index==-1)
    {
        int index = mIdCollection->getAppendIndex (id);

        beginInsertRows (QModelIndex(), index, index);

        mIdCollection->appendRecord (record);

        endInsertRows();
    }
    else
    {
        mIdCollection->replace (index, record);
        emit dataChanged (CSMWorld::IdTable::index (index, 0),
            CSMWorld::IdTable::index (index, mIdCollection->getColumns()-1));
    }
}

const CSMWorld::RecordBase& CSMWorld::IdTable::getRecord (const std::string& id) const
{
    return mIdCollection->getRecord (id);
}

int CSMWorld::IdTable::searchColumnIndex (Columns::ColumnId id) const
{
    return mIdCollection->searchColumnIndex (id);
}

int CSMWorld::IdTable::findColumnIndex (Columns::ColumnId id) const
{
    return mIdCollection->findColumnIndex (id);
}

void CSMWorld::IdTable::reorderRows (int baseIndex, const std::vector<int>& newOrder)
{
    if (!newOrder.empty())
        if (mIdCollection->reorderRows (baseIndex, newOrder))
            emit dataChanged (index (baseIndex, 0),
                index (baseIndex+newOrder.size()-1, mIdCollection->getColumns()-1));
}

std::pair<CSMWorld::UniversalId, std::string> CSMWorld::IdTable::view (int row) const
{
    std::string id;
    std::string hint;

    if (getFeatures() & Feature_ViewCell)
    {
        int cellColumn = mIdCollection->searchColumnIndex (Columns::ColumnId_Cell);
        int idColumn = mIdCollection->searchColumnIndex (Columns::ColumnId_Id);

        if (cellColumn!=-1 && idColumn!=-1)
        {
            id = mIdCollection->getData (row, cellColumn).toString().toUtf8().constData();
            hint = "r:" + std::string (mIdCollection->getData (row, idColumn).toString().toUtf8().constData());
        }
    }
    else if (getFeatures() & Feature_ViewId)
    {
        int column = mIdCollection->searchColumnIndex (Columns::ColumnId_Id);

        if (column!=-1)
        {
            id = mIdCollection->getData (row, column).toString().toUtf8().constData();
            hint = "c:" + id;
        }
    }

    if (id.empty())
        return std::make_pair (UniversalId::Type_None, "");

    if (id[0]=='#')
        id = "sys::default";

    return std::make_pair (UniversalId (UniversalId::Type_Scene, id), hint);
}

///For top level data/columns
bool CSMWorld::IdTable::isDeleted (const std::string& id) const
{
    return getRecord (id).isDeleted();
}

int CSMWorld::IdTable::getColumnId(int column) const
{
    return mIdCollection->getColumn(column).getId();
}

unsigned int CSMWorld::IdTable::foldIndexAdress (const QModelIndex& index) const
{
    unsigned int out = index.row() * this->columnCount();
    out += index.column();
    return ++out;
}

std::pair< int, int > CSMWorld::IdTable::unfoldIndexAdress (unsigned int id) const
{
    if (id == 0)
    {
        throw "Attempt to unfold index id of the top level data cell";
    }

    --id;
    int row = id / this->columnCount();
    int column = id - row * this->columnCount();
    return std::make_pair (row, column);
}

bool CSMWorld::IdTable::hasChildren(const QModelIndex& index) const
{
    return (index.isValid() &&
            index.internalId() == 0 &&
            mIdCollection->getColumn(index.column()).mCanNest &&
            index.data().isValid());
}

void CSMWorld::IdTable::setNestedTable(const QModelIndex& index, const CSMWorld::NestedTableWrapperBase& nestedTable)
{
    if (!hasChildren(index))
    {
        throw std::logic_error("Tried to set nested table, but index has no children");
    }

    bool removeRowsMode = false;
    if (nestedTable.size() != this->nestedTable(index)->size())
    {
        emit resetStart(this->index(index.row(), 0).data().toString());
        removeRowsMode = true;
    }

    dynamic_cast<NestedCollection*>(mIdCollection)->setNestedTable(index.row(), index.column(), nestedTable);

    emit dataChanged (CSMWorld::IdTable::index (index.row(), 0),
                      CSMWorld::IdTable::index (index.row(), mIdCollection->getColumns()-1));

    if (removeRowsMode)
    {
        emit resetEnd(this->index(index.row(), 0).data().toString());
    }
}

CSMWorld::NestedTableWrapperBase* CSMWorld::IdTable::nestedTable(const QModelIndex& index) const
{
    if (!hasChildren(index))
    {
        throw std::logic_error("Tried to retrive nested table, but index has no children");
    }

    return dynamic_cast<NestedCollection*>(mIdCollection)->nestedTable(index.row(), index.column());
}
