-- Auction House Bot - Price History Table
-- Stores daily price snapshots per item per faction

CREATE TABLE IF NOT EXISTS `auctionhouse_price_history` (
  `item_entry` INT UNSIGNED NOT NULL,
  `faction` TINYINT UNSIGNED NOT NULL COMMENT '0=Alliance, 1=Horde, 2=Neutral',
  `min_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `avg_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `median_buyout` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `volume` INT UNSIGNED NOT NULL DEFAULT 0,
  `snapshot_date` DATE NOT NULL,
  PRIMARY KEY (`item_entry`, `faction`, `snapshot_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Daily auction house price history for market analysis';