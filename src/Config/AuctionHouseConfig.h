/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOD_AUCTIONHOUSE_CONFIG_H_
#define MOD_AUCTIONHOUSE_CONFIG_H_

#include "Common.h"
#include "ConfigValueCache.h"
#include <map>
#include <set>
#include <string>
#include <vector>

enum class AHBotConfig
{
    ENABLED,
    UPDATE_INTERVAL,
    MAX_ACTIVE_AUCTIONS_PER_BOT,
    MAX_GOLD_PER_BOT,
    STARTING_GOLD_PER_BOT,
    MIN_PROFIT_MARGIN,
    MAX_BUY_PRICE_PERCENT,
    MIN_SELL_PRICE_PERCENT,
    PRICE_HISTORY_DAYS,
    MIN_ITEM_LEVEL,
    MAX_ITEM_LEVEL,
    ALLOWED_ITEM_CLASSES,
    BLACKLISTED_ITEMS,
    BOT_ACCOUNT_PREFIX,
    BOT_COUNT_PER_FACTION,
    MAX_ITEMS_PER_CYCLE,
    RELIST_THRESHOLD_HOURS,
    UNDERPRICE_THRESHOLD,

    NUM_CONFIGS
};

class AuctionHouseConfig
{
public:
    static AuctionHouseConfig& Instance()
    {
        static AuctionHouseConfig instance;
        return instance;
    }

    void Initialize(bool reload = false);
    void LoadAHBotConfig();

    // AH Bot config getters
    bool IsAHBotEnabled() const { return _ahBotEnabled; }
    uint32 GetUpdateInterval() const { return _updateInterval; }
    uint32 GetMaxActiveAuctionsPerBot() const { return _maxActiveAuctionsPerBot; }
    uint64 GetMaxGoldPerBot() const { return _maxGoldPerBot; }
    uint64 GetStartingGoldPerBot() const { return _startingGoldPerBot; }
    float GetMinProfitMargin() const { return _minProfitMargin; }
    float GetMaxBuyPricePercent() const { return _maxBuyPricePercent; }
    float GetMinSellPricePercent() const { return _minSellPricePercent; }
    uint32 GetPriceHistoryDays() const { return _priceHistoryDays; }
    uint32 GetMinItemLevel() const { return _minItemLevel; }
    uint32 GetMaxItemLevel() const { return _maxItemLevel; }
    const std::set<uint32>& GetAllowedItemClasses() const { return _allowedItemClasses; }
    const std::set<uint32>& GetBlacklistedItems() const { return _blacklistedItems; }
    const std::string& GetBotAccountPrefix() const { return _botAccountPrefix; }
    uint32 GetBotCountPerFaction() const { return _botCountPerFaction; }
    uint32 GetMaxItemsPerCycle() const { return _maxItemsPerCycle; }
    uint32 GetRelistThresholdHours() const { return _relistThresholdHours; }
    float GetUnderpriceThreshold() const { return _underpriceThreshold; }

private:
    AuctionHouseConfig() = default;
    ~AuctionHouseConfig() = default;

    AuctionHouseConfig(const AuctionHouseConfig&) = delete;
    AuctionHouseConfig& operator=(const AuctionHouseConfig&) = delete;

    std::set<uint32> ParseCSVUInt32(const std::string& str) const;

    // AH Bot config
    bool _ahBotEnabled = true;
    uint32 _updateInterval = 300000;
    uint32 _maxActiveAuctionsPerBot = 50;
    uint64 _maxGoldPerBot = 10000000;
    uint64 _startingGoldPerBot = 1000000;
    float _minProfitMargin = 0.15f;
    float _maxBuyPricePercent = 0.80f;
    float _minSellPricePercent = 1.10f;
    uint32 _priceHistoryDays = 30;
    uint32 _minItemLevel = 1;
    uint32 _maxItemLevel = 277;
    std::set<uint32> _allowedItemClasses = {2, 4, 7, 11};
    std::set<uint32> _blacklistedItems;
    std::string _botAccountPrefix = "AHBot_";
    uint32 _botCountPerFaction = 1;
    uint32 _maxItemsPerCycle = 20;
    uint32 _relistThresholdHours = 12;
    float _underpriceThreshold = 0.50f;
};

#define sAuctionHouseConfig AuctionHouseConfig::Instance()

#endif