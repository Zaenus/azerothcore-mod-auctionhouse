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

#ifndef MOD_AUCTIONHOUSE_MARKET_ANALYZER_H_
#define MOD_AUCTIONHOUSE_MARKET_ANALYZER_H_

#include "Common.h"
#include "DatabaseEnv.h"
#include "AuctionHouseMgr.h"
#include <mutex>
#include <unordered_map>

struct MarketPriceData
{
    uint64 minBuyout = 0;
    uint64 avgBuyout = 0;
    uint64 medianBuyout = 0;
    uint32 volume = 0;
    time_t lastUpdate = 0;
    bool hasData = false;
};

class MarketAnalyzer
{
public:
    static MarketAnalyzer& Instance()
    {
        static MarketAnalyzer instance;
        return instance;
    }

    void Initialize();
    void UpdatePrices();

    // Called from AuctionHouseScript hooks
    void RecordListing(AuctionHouseObject* ah, AuctionEntry* entry);
    void RecordSale(AuctionHouseObject* ah, AuctionEntry* entry);
    void RecordExpiry(AuctionHouseObject* ah, AuctionEntry* entry);

    // Query market data
    MarketPriceData GetMarketPrice(uint32 itemEntry, AuctionHouseFaction faction) const;
    float GetPriceTrend(uint32 itemEntry, AuctionHouseFaction faction, uint32 days = 7) const;
    uint64 GetMarketValue(uint32 itemEntry, AuctionHouseFaction faction) const;

    // Daily snapshot (called at midnight)
    void DailySnapshot();

    // Cleanup old history
    void CleanupOldHistory(uint32 daysToKeep);

private:
    MarketAnalyzer() = default;
    ~MarketAnalyzer() = default;

    MarketAnalyzer(const MarketAnalyzer&) = delete;
    MarketAnalyzer& operator=(const MarketAnalyzer&) = delete;

    struct PriceKey
    {
        uint32 itemEntry;
        AuctionHouseFaction faction;

        bool operator==(const PriceKey& other) const
        {
            return itemEntry == other.itemEntry && faction == other.faction;
        }
    };

    struct PriceKeyHash
    {
        size_t operator()(const PriceKey& key) const noexcept
        {
            return (static_cast<size_t>(key.itemEntry) << 2) ^ static_cast<size_t>(key.faction);
        }
    };

    mutable std::shared_mutex _pricesLock;
    std::unordered_map<PriceKey, MarketPriceData, PriceKeyHash> _cachedPrices;

    void LoadPriceHistory();
    void SavePriceSnapshot(uint32 itemEntry, AuctionHouseFaction faction, const MarketPriceData& data);
    void CalculateMedian(std::vector<uint64>& buyouts, uint64& median) const;

    static AuctionHouseFaction GetFactionFromAH(AuctionHouseObject* ah);
};

#endif