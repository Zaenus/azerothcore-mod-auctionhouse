# mod-auctionhouse

An [AzerothCore](https://www.azerothcore.org/) module that enhances the in-game Auction House with two complementary systems:

- **Auction House Bot** – an economy-balancing bot that automatically buys and sells items to keep the AH populated and prices fair.
- **Black Market Auction House (BMAH)** – a rotating, high-stakes AH for rare mounts, pets, transmog gear, and other exclusive items.

## Features

### Auction House Bot
- Spawns configurable bot characters per faction (Alliance, Horde, Neutral).
- Analyzes market prices using a rolling price-history window.
- Buys underpriced items and relists them at a profit.
- Posts items from its inventory at a percentage above market value.
- Fully configurable update interval, gold limits, item-level range, allowed item classes, and blacklisted items.

### Black Market Auction House
- Weekly (configurable) rotation of exclusive items across weighted item pools:
  - Rare mounts, companion pets, transmog/T3 gear, high-end BiS offset pieces, TCG items, and miscellaneous rares.
- Configurable auction durations, bid increments, deposit, and AH cut.
- Custom NPC auctioneer entry.

## Requirements

- [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) (latest `master` branch recommended)
- CMake 3.14+
- A C++17-capable compiler

## Installation

1. Clone this repository into your AzerothCore `modules/` directory:

   ```bash
   cd /path/to/azerothcore/modules
   git clone https://github.com/Zaenus/azerothcore-mod-auctionhouse mod-auctionhouse
   ```

2. Re-run CMake to pick up the new module:

   ```bash
   cmake ..
   ```

3. Build the server as usual.

4. Copy the default configuration file to your worldserver config directory:

   ```bash
   cp modules/mod-auctionhouse/conf/auctionhouse.conf.dist worldserver/configs/auctionhouse.conf
   ```

5. (Re)start the worldserver.

## Configuration

All settings are in `auctionhouse.conf`. A fully-annotated template is provided in [`conf/auctionhouse.conf.dist`](conf/auctionhouse.conf.dist).

### Auction House Bot

| Option | Default | Description |
|--------|---------|-------------|
| `AuctionHouseBot.Enabled` | `1` | Enable/disable the bot |
| `AuctionHouseBot.UpdateInterval` | `300000` | Bot update cycle in ms (default: 5 min) |
| `AuctionHouseBot.BotCountPerFaction` | `1` | Number of bots per faction |
| `AuctionHouseBot.MaxActiveAuctionsPerBot` | `50` | Max simultaneous listings per bot |
| `AuctionHouseBot.StartingGoldPerBot` | `1000000` | Starting gold in copper (100g) |
| `AuctionHouseBot.MaxGoldPerBot` | `10000000` | Gold cap in copper (1000g) |
| `AuctionHouseBot.MinProfitMargin` | `0.15` | Minimum profit margin (15%) |
| `AuctionHouseBot.MaxBuyPricePercent` | `0.80` | Max fraction of market value to pay when buying |
| `AuctionHouseBot.MinSellPricePercent` | `1.10` | Min fraction of market value when selling |
| `AuctionHouseBot.PriceHistoryDays` | `30` | Days of price history to retain |
| `AuctionHouseBot.MinItemLevel` | `1` | Minimum item level considered |
| `AuctionHouseBot.MaxItemLevel` | `277` | Maximum item level considered |
| `AuctionHouseBot.AllowedItemClasses` | `"2,4,7,11"` | Item classes the bot trades |
| `AuctionHouseBot.BlacklistedItems` | `""` | Comma-separated item entries to never trade |
| `AuctionHouseBot.MaxItemsPerCycle` | `20` | Max buy/sell operations per update |
| `AuctionHouseBot.RelistThresholdHours` | `12` | Hours before relisting unsold items |
| `AuctionHouseBot.UnderpriceThreshold` | `0.50` | Buy if item is below this fraction of market value |

### Black Market Auction House

| Option | Default | Description |
|--------|---------|-------------|
| `BlackMarketAH.Enabled` | `1` | Enable/disable the BMAH |
| `BlackMarketAH.NPCEntry` | `67766` | Creature entry for the BMAH auctioneer |
| `BlackMarketAH.RefreshDay` | `3` | Day of week for rotation (0=Sun, 3=Wed) |
| `BlackMarketAH.RefreshHour` | `3` | Hour of day for rotation (server time) |
| `BlackMarketAH.MaxActiveAuctions` | `10` | Max concurrent BMAH listings |
| `BlackMarketAH.MinDuration` | `86400` | Min auction duration in seconds (24h) |
| `BlackMarketAH.MaxDuration` | `172800` | Max auction duration in seconds (48h) |
| `BlackMarketAH.BidIncrementPercent` | `0.05` | Minimum bid increment (5%) |
| `BlackMarketAH.DepositMultiplier` | `0.0` | Deposit multiplier (0 = free) |
| `BlackMarketAH.CutPercent` | `0` | AH cut percentage (0 = none) |

Item pools (`BlackMarketAH.Pool.*`) are configurable with a `Weight:MinIlvl:MaxIlvl:ItemEntries` format. See the config file for the full defaults.

## License

Copyright © 2024+ AzerothCore. Released under the [GNU AGPL v3](LICENSE) license.
