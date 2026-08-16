-- Auction House Module - Character Database Updates
-- Creates tables for AH Bot price history and inventory

-- auctionhouse_price_history
CREATE TABLE `auctionhouse_price_history` (
  `item_entry` INT UNSIGNED NOT NULL,
  `faction` TINYINT UNSIGNED NOT NULL COMMENT '0=Alliance, 1=Horde, 2=Neutral',
  `min_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `avg_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `median_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `volume` INT UNSIGNED NOT NULL DEFAULT 0,
  `snapshot_date` DATE NOT NULL,
  PRIMARY KEY (`item_entry`, `faction`, `snapshot_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Daily auction house price history for market analysis';

-- auctionhouse_bot_inventory
CREATE TABLE `auctionhouse_bot_inventory` (
  `bot_guid` BIGINT UNSIGNED NOT NULL,
  `item_guid` BIGINT UNSIGNED NOT NULL,
  `item_entry` INT UNSIGNED NOT NULL,
  `count` INT UNSIGNED NOT NULL DEFAULT 1,
  `acquired_price` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `acquired_date` DATE NOT NULL,
  `listed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `auction_id` INT UNSIGNED DEFAULT NULL,
  PRIMARY KEY (`bot_guid`, `item_guid`),
  KEY `idx_bot_entry` (`bot_guid`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Items owned by auction house bots';