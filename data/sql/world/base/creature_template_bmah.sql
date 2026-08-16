-- Black Market Auction House - NPC Spawn
-- Spawns the Black Market Auctioneer in Dalaran Sewers (map 571)
-- Near the existing "TheBlackMarket" teleporter at ~5922, 566, 609

-- Black Market Auctioneer (custom entry 67766)
DELETE FROM `creature_template` WHERE `entry` = 67766;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattacktime`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(67766, 'Black Market Auctioneer', 'Black Market Auction House', 'Auctioneer', 0, 90, 90, 0, 35, 1, 1.14286, 1, 1, 0, 0, 1, 2000, 0, 1, 33554432, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 138936320, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.5625, 1, 1, 1, 0, 65085440, 2, 'npc_blackmarket_auctioneer', 12340);

-- Model for the NPC (Auctioneer display ID 39776)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 67766;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(67766, 0, 39776, 1, 1, 12340);

-- Spawn in Dalaran Sewers (map 571) near TheBlackMarket teleporter
DELETE FROM `creature` WHERE `id` = 67766;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `ScriptName`) VALUES
(1, 67766, 571, 4395, 4395, 1, 1, 0, 5922.4, 566.631, 609.825, 5.58114, 300, 0, 0, 1073741824, 0, 0, 'npc_blackmarket_auctioneer');