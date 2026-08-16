-- Auction House Bot - Bot Inventory Table
-- Tracks items owned by AH bots

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