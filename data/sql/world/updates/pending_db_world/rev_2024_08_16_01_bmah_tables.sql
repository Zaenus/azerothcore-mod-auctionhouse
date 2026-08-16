-- Auction House Module - World Database Updates
-- Creates tables for Black Market AH

-- blackmarket_auctions
CREATE TABLE `blackmarket_auctions` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `item_entry` INT UNSIGNED NOT NULL,
  `item_guid` BIGINT UNSIGNED NOT NULL,
  `start_bid` BIGINT UNSIGNED NOT NULL,
  `current_bid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `bidder_guid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `expire_time` TIMESTAMP NOT NULL,
  `pool_category` VARCHAR(50) NOT NULL,
  `rotation_id` INT UNSIGNED NOT NULL,
  `created_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_expire` (`expire_time`),
  KEY `idx_rotation` (`rotation_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Active Black Market Auction House auctions';

-- blackmarket_rotation_history
CREATE TABLE `blackmarket_rotation_history` (
  `rotation_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `start_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `end_time` TIMESTAMP NULL DEFAULT NULL,
  `items_json` TEXT NOT NULL COMMENT 'JSON array of items in rotation',
  PRIMARY KEY (`rotation_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Black Market Auction House rotation history';

-- blackmarket_item_pools
CREATE TABLE `blackmarket_item_pools` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `category` VARCHAR(50) NOT NULL COMMENT 'Category: Mounts, Pets, Transmog, Gear, TCG, Misc',
  `item_entry` INT UNSIGNED NOT NULL,
  `weight` INT UNSIGNED NOT NULL DEFAULT 1,
  `min_ilvl` INT UNSIGNED NOT NULL DEFAULT 1,
  `max_ilvl` INT UNSIGNED NOT NULL DEFAULT 277,
  `min_bid` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_category_item` (`category`, `item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Black Market Auction House item pools';