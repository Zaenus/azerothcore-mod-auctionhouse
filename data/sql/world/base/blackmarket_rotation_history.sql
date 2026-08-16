-- Black Market Auction House - Rotation History Table

CREATE TABLE `blackmarket_rotation_history` (
  `rotation_id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `start_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `end_time` TIMESTAMP NULL DEFAULT NULL,
  `items_json` TEXT NOT NULL COMMENT 'JSON array of items in rotation',
  PRIMARY KEY (`rotation_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Black Market Auction House rotation history';