-- Black Market Auction House - Populate Item Pools from Config
-- Run on module enable or config change

-- Mounts (Weight: 100, MinIlvl: 1, MaxIlvl: 277)
INSERT IGNORE INTO `blackmarket_item_pools` (`category`, `item_entry`, `weight`, `min_ilvl`, `max_ilvl`, `min_bid`, `enabled`) VALUES
('Mounts', 35513, 100, 1, 277, 5000000, 1),    -- Reins of the Bronze Drake
('Mounts', 44151, 100, 1, 277, 10000000, 1),   -- Blue Proto-Drake
('Mounts', 44152, 100, 1, 277, 50000000, 1),   -- Time-Lost Proto-Drake
('Mounts', 44153, 100, 1, 277, 10000000, 1),   -- Red Proto-Drake
('Mounts', 44154, 100, 1, 277, 10000000, 1),   -- Green Proto-Drake
('Mounts', 44156, 100, 1, 277, 10000000, 1),   -- Violet Proto-Drake
('Mounts', 44160, 100, 1, 277, 10000000, 1),   -- Plagued Proto-Drake
('Mounts', 44164, 100, 1, 277, 10000000, 1),   -- Albino Drake
('Mounts', 44177, 100, 1, 277, 10000000, 1),   -- Green Proto-Drake (alt)
('Mounts', 44178, 100, 1, 277, 10000000, 1),   -- Albino Drake (alt)
('Mounts', 45693, 100, 1, 277, 100000000, 1),  -- Mimiron's Head
('Mounts', 45802, 100, 1, 277, 500000000, 1),  -- Invincible
('Mounts', 43954, 100, 1, 277, 20000000, 1),   -- Grand Black War Mammoth (A)
('Mounts', 43956, 100, 1, 277, 20000000, 1),   -- Grand Black War Mammoth (H)
('Mounts', 43959, 100, 1, 277, 20000000, 1),   -- Grand Black War Mammoth
('Mounts', 43961, 100, 1, 277, 20000000, 1),   -- Ice Mammoth
('Mounts', 43963, 100, 1, 277, 20000000, 1),   -- Ice Mammoth
('Mounts', 43965, 100, 1, 277, 20000000, 1),   -- Traveler's Tundra Mammoth
('Mounts', 43967, 100, 1, 277, 20000000, 1),   -- Traveler's Tundra Mammoth
('Mounts', 43969, 100, 1, 277, 20000000, 1),   -- Wooly Mammoth
('Mounts', 43971, 100, 1, 277, 20000000, 1),   -- Wooly Mammoth
('Mounts', 43973, 100, 1, 277, 20000000, 1),   -- Black War Bear
('Mounts', 43975, 100, 1, 277, 20000000, 1),   -- Black War Bear
('Mounts', 43977, 100, 1, 277, 20000000, 1),   -- Swift Horde Wolf
('Mounts', 43979, 100, 1, 277, 20000000, 1),   -- Swift Alliance Steed
('Mounts', 43981, 100, 1, 277, 20000000, 1),   -- Swift Horde Wolf
('Mounts', 43983, 100, 1, 277, 20000000, 1),   -- Swift Alliance Steed
('Mounts', 43985, 100, 1, 277, 20000000, 1),   -- Red Proto-Drake
('Mounts', 43987, 100, 1, 277, 20000000, 1),   -- Blue Proto-Drake
('Mounts', 43989, 100, 1, 277, 20000000, 1),   -- Time-Lost Proto-Drake
('Mounts', 43991, 100, 1, 277, 20000000, 1);   -- Plagued Proto-Drake

-- Pets (Weight: 50, MinIlvl: 1, MaxIlvl: 277)
INSERT IGNORE INTO `blackmarket_item_pools` (`category`, `item_entry`, `weight`, `min_ilvl`, `max_ilvl`, `min_bid`, `enabled`) VALUES
('Pets', 44822, 50, 1, 277, 5000000, 1),   -- Wolpertinger
('Pets', 44738, 50, 1, 277, 5000000, 1),   -- Egbert
('Pets', 44820, 50, 1, 277, 5000000, 1),   -- Pepe
('Pets', 44732, 50, 1, 277, 3000000, 1),   -- Gundrak Hatchling
('Pets', 44733, 50, 1, 277, 3000000, 1),   -- Tickbird Hatchling
('Pets', 44734, 50, 1, 277, 3000000, 1),   -- Cobra Hatchling
('Pets', 44735, 50, 1, 277, 3000000, 1),   -- White Tickbird Hatchling
('Pets', 44736, 50, 1, 277, 3000000, 1),   -- Darting Hatchling
('Pets', 44737, 50, 1, 277, 3000000, 1),   -- Razormaw Hatchling
('Pets', 44739, 50, 1, 277, 3000000, 1),   -- Razzashi Hatchling
('Pets', 44740, 50, 1, 277, 3000000, 1),   -- Deviate Hatchling
('Pets', 44741, 50, 1, 277, 3000000, 1),   -- Leaping Hatchling
('Pets', 44742, 50, 1, 277, 3000000, 1),   -- Ravasaur Hatchling
('Pets', 44743, 50, 1, 277, 3000000, 1);   -- Razormaw Hatchling (alt)

-- Transmog: Legendaries (Weight: 80)
INSERT IGNORE INTO `blackmarket_item_pools` (`category`, `item_entry`, `weight`, `min_ilvl`, `max_ilvl`, `min_bid`, `enabled`) VALUES
('Transmog', 19019, 80, 1, 277, 100000000, 1), -- Thunderfury
('Transmog', 17182, 80, 1, 277, 100000000, 1), -- Sulfuras
('Transmog', 17193, 80, 1, 277, 100000000, 1), -- Sulfuras (offhand)
('Transmog', 32837, 80, 1, 277, 100000000, 1), -- Warglaive of Azzinoth (MH)
('Transmog', 32838, 80, 1, 277, 100000000, 1), -- Warglaive of Azzinoth (OH)
('Transmog', 46017, 80, 1, 277, 100000000, 1), -- Val'anyr
('Transmog', 49623, 80, 1, 277, 500000000, 1), -- Shadowmourne

-- Gear: BIS Offset (Weight: 30, MinIlvl: 200)
('Gear', 50425, 30, 200, 277, 5000000, 1),
('Gear', 50426, 30, 200, 277, 5000000, 1),
('Gear', 50427, 30, 200, 277, 5000000, 1),
('Gear', 50428, 30, 200, 277, 5000000, 1),
('Gear', 50429, 30, 200, 277, 5000000, 1),
('Gear', 50430, 30, 200, 277, 5000000, 1);

-- TCG (Weight: 5)
INSERT IGNORE INTO `blackmarket_item_pools` (`category`, `item_entry`, `weight`, `min_ilvl`, `max_ilvl`, `min_bid`, `enabled`) VALUES
('TCG', 49282, 5, 1, 277, 100000000, 1), -- Spectral Tiger
('TCG', 49283, 5, 1, 277, 100000000, 1), -- Magic Rooster
('TCG', 49284, 5, 1, 277, 100000000, 1), -- Silver Riding Nether Ray
('TCG', 49285, 5, 1, 277, 100000000, 1), -- Purple Riding Nether Ray
('TCG', 49286, 5, 1, 277, 100000000, 1), -- Red Riding Nether Ray
('TCG', 49287, 5, 1, 277, 100000000, 1), -- Blue Riding Nether Ray
('TCG', 49288, 5, 1, 277, 100000000, 1), -- Green Riding Nether Ray
('TCG', 49289, 5, 1, 277, 100000000, 1), -- Swift Spectral Tiger
('TCG', 49290, 5, 1, 277, 100000000, 1); -- X-51 Nether-Rocket

-- Misc (Weight: 20)
INSERT IGNORE INTO `blackmarket_item_pools` (`category`, `item_entry`, `weight`, `min_ilvl`, `max_ilvl`, `min_bid`, `enabled`) VALUES
('Misc', 43959, 20, 1, 277, 20000000, 1), -- Grand Black War Mammoth
('Misc', 44231, 20, 1, 277, 20000000, 1), -- Traveler's Tundra Mammoth
('Misc', 43010, 20, 1, 277, 1000000, 1),  -- Kirin Tor Ring
('Misc', 43011, 20, 1, 277, 1000000, 1),  -- Wyrmrest Accord Ring
('Misc', 43012, 20, 1, 277, 1000000, 1),  -- Argent Crusade Ring
('Misc', 43013, 20, 1, 277, 1000000, 1),  -- Knights of the Ebon Blade Ring
('Misc', 43014, 20, 1, 277, 1000000, 1),  -- Kirin Tor Ring
('Misc', 43015, 20, 1, 277, 1000000, 1),  -- Wyrmrest Accord Ring
('Misc', 43015, 20, 1, 277, 1000000, 1),  -- Argent Crusade Ring
('Misc', 43016, 20, 1, 277, 1000000, 1);  -- Knights of the Ebon Blade Ring