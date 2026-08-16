-- Prepared Statements for Auction House Module
-- Add to CharacterDatabasePreparedStatements.h and WorldDatabasePreparedStatements.h

-- ============================================================
-- CHARACTER DATABASE STATEMENTS
-- ============================================================

-- Auction House Price History
CHAR_SEL_AUCTION_PRICE_HISTORY = "SELECT item_entry, faction, min_buyout, avg_buyout, median_buyout, volume FROM auctionhouse_price_history"
CHAR_INS_AUCTION_PRICE_HISTORY = "INSERT INTO auctionhouse_price_history (item_entry, faction, min_buyout, avg_buyout, median_buyout, volume, snapshot_date) VALUES (?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE min_buyout=VALUES(min_buyout), avg_buyout=VALUES(avg_buyout), median_buyout=VALUES(median_buyout), volume=VALUES(volume)"
CHAR_DEL_OLD_AUCTION_PRICE_HISTORY = "DELETE FROM auctionhouse_price_history WHERE snapshot_date < DATE_SUB(CURDATE(), INTERVAL ? DAY)"
CHAR_SEL_AUCTION_PRICE_TREND = "SELECT AVG(avg_buyout) as old_avg FROM auctionhouse_price_history WHERE item_entry = ? AND faction = ? AND snapshot_date < DATE_SUB(CURDATE(), INTERVAL ? DAY) UNION SELECT AVG(avg_buyout) as new_avg FROM auctionhouse_price_history WHERE item_entry = ? AND faction = ? AND snapshot_date >= DATE_SUB(CURDATE(), INTERVAL ? DAY)"

-- AH Bot Inventory
CHAR_SEL_AH_BOT_INVENTORY = "SELECT bot_guid, item_entry, count, acquired_price, acquired_date, listed, auction_id FROM auctionhouse_bot_inventory WHERE bot_guid = ?"
CHAR_INS_AH_BOT_INVENTORY = "INSERT INTO auctionhouse_bot_inventory (bot_guid, item_guid, item_entry, count, acquired_price, acquired_date, listed, auction_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE count=VALUES(count), acquired_price=VALUES(acquired_price), listed=VALUES(listed), auction_id=VALUES(auction_id)"
CHAR_UPD_AH_BOT_INVENTORY = "UPDATE auctionhouse_bot_inventory SET count = ?, listed = ?, auction_id = ? WHERE bot_guid = ? AND item_guid = ?"
CHAR_DEL_AH_BOT_INVENTORY = "DELETE FROM auctionhouse_bot_inventory WHERE bot_guid = ? AND item_guid = ?"

-- Character Money (existing)
CHAR_SEL_CHARACTER_MONEY = "SELECT money FROM characters WHERE guid = ?"
CHAR_UPD_CHARACTER_MONEY = "UPDATE characters SET money = ? WHERE guid = ?"

-- ============================================================
-- WORLD DATABASE STATEMENTS
-- ============================================================

-- BMAH Item Pools
WORLD_SEL_BMAH_ITEM_POOLS = "SELECT id, category, item_entry, weight, min_ilvl, max_ilvl, min_bid, enabled FROM blackmarket_item_pools WHERE enabled = 1 ORDER BY category, weight DESC"

-- BMAH Auctions
WORLD_SEL_BMAH_AUCTIONS = "SELECT id, item_entry, item_guid, start_bid, current_bid, bidder_guid, expire_time, pool_category, rotation_id FROM blackmarket_auctions WHERE expire_time > UNIX_TIMESTAMP()"
WORLD_INS_BMAH_AUCTION = "INSERT INTO blackmarket_auctions (item_entry, item_guid, start_bid, current_bid, bidder_guid, expire_time, pool_category, rotation_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
WORLD_UPD_BMAH_AUCTION = "UPDATE blackmarket_auctions SET current_bid = ?, bidder_guid = ? WHERE id = ?"
WORLD_DEL_BMAH_AUCTION = "DELETE FROM blackmarket_auctions WHERE id = ?"
WORLD_DEL_EXPIRED_BMAH_AUCTIONS = "DELETE FROM blackmarket_auctions WHERE expire_time <= UNIX_TIMESTAMP()"

-- BMAH Rotation History
WORLD_INS_BMAH_ROTATION_HISTORY = "INSERT INTO blackmarket_rotation_history (rotation_id, items_json) VALUES (?, ?)"
WORLD_SEL_BMAH_LAST_ROTATION = "SELECT rotation_id FROM blackmarket_rotation_history ORDER BY rotation_id DESC LIMIT 1"
WORLD_UPD_BMAH_LAST_ROTATION = "INSERT INTO blackmarket_rotation_history (rotation_id) VALUES (?) ON DUPLICATE KEY UPDATE rotation_id = VALUES(rotation_id)"