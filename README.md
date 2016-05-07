OpenMW-MCP
==========

This is a fork of [https://github.com/OpenMW/openmw](OpenMW).

OpenMW is a free implementation of The Elder Scrolls III: Morrowind's
game engine that is compatible with the original game files and many
of the community-created mods.

**OpenMW-MCP** is a fork that implements some features provided by the
[http://www.uesp.net/wiki/Tes3Mod:Morrowind_Code_Patch](Morrowind Code
Patch) on top of OpenMW. A lot of the functionality of the MCP is just
bugfixes for the vanilla engine. These are not a problem for OpenMW as
it simply didn't implement those bugs. However, MCP also added some
original features, which will probably not find their way into OpenMW
as it tries to keep close to vanilla Morrowind.

## Features

Here is a list of features that this fork currently implements:

- *Swiftcasting*. The ability to cast a spell immediately despite the
  player not having their hands ready for spellcasting. The button for
  swiftcasting should be set in `Settings -> Controls`, the default is `Z`.
- *Fairer pickpocketing*. The formula for calculating the chance of
  getting caught when pickpocketing an item is broken and can result
  in negative values. It is replaces with a simpler one that just
  takes the player's Sneak skill into account.
- *Pickpocketing knocked out NPCs*.
- *Hidden traps*. Trapped doors and containers will not say they are
  trapped unless your Security skill is high enough. At skill 0 all
  traps will be invisible to the player, at skill 100 none will be.
- *Spellmaking ranges*. When creating spells, the magnitude of spells
  has been increased to 500 from 100 (so it's more useful for effects
  like Feather), and the duration has been decreased from 1440 to 300.
  The latter makes the slider behave more smoothly.
- *Strength-based hand-to-hand damage*. The original game only takes
  the attacker's hand-to-hand skill into account when calculating
  damage. MCP also takes into account their Strength by applying a
  modifier: 1.0x at Strength 40, 2.5x at Strength 100.
- *Detect Animal detects all life*. This just makes it detect NPCs as
  well as creatures.

## FAQ

### Are there ready-made builds of OpenMW-MCP?

No.

Your best bet is to clone this repository and build it.
See [https://wiki.openmw.org/index.php?title=Development_Environment_Setup]().

Don't forget to checkout the `mcp` branch before building to get the
MCP features.

### Can I use only some of these features?

Yes, if you clone this repository and revert the unneeded commits.
Alternatively, extract the required commits as patches and apply them
to OpenMW's master branch before building.

### How does this relate to OpenMW?

I'm not involved with OpenMW development at all. This fork is
something I do on my own.

### I built OpenMW-MCP, but I don't see any of the MCP features.

Make sure you checkout `mcp` branch before building. The `master`
branch is identical to upstream `master`.
