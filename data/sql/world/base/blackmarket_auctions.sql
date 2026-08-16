-- Black Market Auction House - Active Auctions Table

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