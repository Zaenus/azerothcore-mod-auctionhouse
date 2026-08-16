-- Black Market Auction House - NPC Spawn
-- Spawns the Black Market Auctioneer in Dalaran Sewers (map 571)
-- Near the existing "TheBlackMarket" teleporter at ~5922, 566, 609

-- Black Market Auctioneer (custom entry 67766)
DELETE FROM `creature_template` WHERE `entry` = 67766;
INSERT INTO `creature_template` (`entry`, `modelid_1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `damage_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattacktime`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `petspell1`, `petspell2`, `petspell3`, `petspell4`, `vehicleid`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Health_mod`, `Mana_mod`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES
(67766, 39776, 'Black Market Auctioneer', 'Black Market Auction House', 'Auctioneer', 0, 90, 90, 0, 1, 1.14286, 1, 1, 0, 0, 1, 2000, 0, 1, 33554432, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 138936320, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, '', 0, 3, 1, 1.5625, 1, 1, 1, 0, 65085440, 2, 'npc_blackmarket_auctioneer');

-- Spawn in Dalaran Sewers (map 571) near TheBlackMarket teleporter
DELETE FROM `creature` WHERE `id` = 67766;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `DeathState`, `MovementType`) VALUES
(1, 67766, 571, 4395, 4395, 1, 1, 0, 0, 5922.4, 566.631, 609.825, 5.58114, 300, 0, 0, 1073741824, 0, 0, 0);