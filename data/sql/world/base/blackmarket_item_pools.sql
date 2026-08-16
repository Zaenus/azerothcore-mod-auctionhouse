-- Black Market Auction House - Item Pools Table
-- Defines weighted item pools for BMAH rotations

CREATE TABLE IF NOT EXISTS `blackmarket_item_pools` (
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