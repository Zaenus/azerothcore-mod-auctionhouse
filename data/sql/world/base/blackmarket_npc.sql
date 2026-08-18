-- Black Market Auctioneer NPC for mod-auctionhouse
-- Adds creature_template entry and sets ethereal model (Consortium style)
-- Adds custom gossip text: "Is that all stranger? He-He, thank you."

-- Cleanup existing entries
DELETE FROM `creature_template` WHERE `entry` = 67766;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 67766;
DELETE FROM `broadcast_text` WHERE `ID` IN (1000000, 1000001);
DELETE FROM `broadcast_text_locale` WHERE `ID` IN (1000000, 1000001);

-- creature_template: Black Market Auctioneer
-- Neutral faction (35), Gossip + Auctioneer NPC flags, level 85
-- Display set via creature_template_model to ethereal model
INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`,
    `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`,
    `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`,
    `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`,
    `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES (
    67766, 0, 0, 0,
    0, 0, 'Black Market Auctioneer', 'Black Market', 'Directions', 0,
    85, 85, 0, 35, 268435457, 1.0, 1.14286,
    1.0, 1.0, 18.0, 0, 0,
    1.0, 2000, 2000, 1.0, 1.0,
    1, 32768, 0, 0, 0, 7,
    138936390, 0, 0, 0, 0, 0,
    0, 0, '', 0, 1.0, 1.56,
    1.0, 1.0, 1.0, 0, 0,
    1, 0, 0, 'npc_blackmarket_auctioneer', 12340
);

-- creature_template_model: Set ethereal male blue display (Consortium style)
-- DisplayID 20325 = Ethereal Male (Blue) - typical Consortium appearance
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(67766, 0, 20325, 1.0, 1.0, 12340);

-- Custom gossip text for the Black Market Auctioneer
-- ID 1000000: "Is that all stranger? He-He, thank you."
INSERT INTO `broadcast_text` (`ID`, `LanguageID`, `MaleText`, `FemaleText`, `EmoteID1`, `EmoteID2`, `EmoteID3`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `SoundEntriesId`, `EmotesID`, `Flags`, `VerifiedBuild`) VALUES
(1000000, 0, 'Is that all stranger? He-He, thank you.', 'Is that all stranger? He-He, thank you.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 12340);

-- Locale support (enUS)
INSERT INTO `broadcast_text_locale` (`ID`, `locale`, `MaleText`, `FemaleText`, `VerifiedBuild`) VALUES
(1000000, 'enUS', 'Is that all stranger? He-He, thank you.', 'Is that all stranger? He-He, thank you.', 12340);