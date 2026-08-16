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

#include "MarketAnalyzer.h"
#include "AuctionHouseMgr.h"
#include "CharacterDatabase.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "QueryResult.h"
#include <algorithm>

void MarketAnalyzer::Initialize()
{
    _initialized = false;
    LOG_INFO("modules.auctionhouse", "MarketAnalyzer initialized");
}

void MarketAnalyzer::EnsureLoaded() const
{
    if (_initialized)
        return;

    std::unique_lock lock(_initLock);
    if (_initialized)
        return;

    LoadPriceHistory();
    _initialized = true;
    LOG_INFO("modules.auctionhouse", "MarketAnalyzer loaded with {} cached prices", _cachedPrices.size());
}

void MarketAnalyzer::LoadPriceHistory() const
{
    std::unique_lock lock(_pricesLock);

    // Use raw query since prepared statements don't exist in core
    QueryResult result = CharacterDatabase.Query("SELECT item_entry, faction, min_buyout, avg_buyout, median_buyout, volume FROM auctionhouse_price_history");

    if (!result)
    {
        LOG_INFO("modules.auctionhouse", "No price history found in database");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 itemEntry = fields[0].Get<uint32>();
        uint8 faction = fields[1].Get<uint8>();
        uint64 minBuyout = fields[2].Get<uint64>();
        uint64 avgBuyout = fields[3].Get<uint64>();
        uint64 medianBuyout = fields[4].Get<uint64>();
        uint32 volume = fields[5].Get<uint32>();

        PriceKey key{itemEntry, static_cast<AuctionHouseFaction>(faction)};
        MarketPriceData data;
        data.minBuyout = minBuyout;
        data.avgBuyout = avgBuyout;
        data.medianBuyout = medianBuyout;
        data.volume = volume;
        data.hasData = true;

        _cachedPrices[key] = data;
        ++count;
    } while (result->NextRow());

    LOG_INFO("modules.auctionhouse", "Loaded {} price history records", count);
}

void MarketAnalyzer::UpdatePrices()
{
    std::unique_lock lock(_pricesLock);

    for (uint8 factionIdx = 0; factionIdx < 3; ++factionIdx)
    {
        AuctionHouseFaction faction = static_cast<AuctionHouseFaction>(factionIdx);
        AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(
            faction == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
            faction == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

        if (!ah || ah->Getcount() == 0)
            continue;

        std::unordered_map<uint32, std::vector<uint64>> itemBuyouts;
        std::unordered_map<uint32, uint32> itemVolumes;

        for (auto const& [id, auction] : ah->GetAuctions())
        {
            if (auction->buyout > 0)
            {
                itemBuyouts[auction->item_template].push_back(auction->buyout);
                ++itemVolumes[auction->item_template];
            }
        }

        for (auto const& [itemEntry, buyouts] : itemBuyouts)
        {
            if (buyouts.empty())
                continue;

            uint64 minBuyout = *std::min_element(buyouts.begin(), buyouts.end());
            uint64 sum = 0;
            for (uint64 b : buyouts)
                sum += b;
            uint64 avgBuyout = sum / buyouts.size();

            uint64 medianBuyout = 0;
            CalculateMedian(buyouts, medianBuyout);

            PriceKey key{itemEntry, faction};
            MarketPriceData data;
            data.minBuyout = minBuyout;
            data.avgBuyout = avgBuyout;
            data.medianBuyout = medianBuyout;
            data.volume = itemVolumes[itemEntry];
            data.lastUpdate = time(nullptr);
            data.hasData = true;

            _cachedPrices[key] = data;
        }
    }
}

void MarketAnalyzer::RecordListing(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!entry || entry->buyout == 0)
        return;

    EnsureLoaded();

    AuctionHouseFaction faction = GetFactionFromAH(ah);
    PriceKey key{entry->item_template, faction};

    std::shared_lock readLock(_pricesLock);
    auto it = _cachedPrices.find(key);
    readLock.unlock();

    if (it != _cachedPrices.end() && it->second.hasData)
    {
        std::unique_lock writeLock(_pricesLock);
        MarketPriceData& data = it->second;
        data.volume = std::min<uint32>(data.volume + 1, 10000);
        data.lastUpdate = time(nullptr);
    }
}

void MarketAnalyzer::RecordSale(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!entry || entry->buyout == 0)
        return;

    EnsureLoaded();

    AuctionHouseFaction faction = GetFactionFromAH(ah);
    PriceKey key{entry->item_template, faction};

    std::shared_lock readLock(_pricesLock);
    auto it = _cachedPrices.find(key);
    readLock.unlock();

    if (it != _cachedPrices.end() && it->second.hasData)
    {
        std::unique_lock writeLock(_pricesLock);
        MarketPriceData& data = it->second;
        data.volume = std::min<uint32>(data.volume + 1, 10000);
        data.lastUpdate = time(nullptr);
    }
}

void MarketAnalyzer::RecordExpiry(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!entry || entry->buyout == 0)
        return;

    EnsureLoaded();

    AuctionHouseFaction faction = GetFactionFromAH(ah);
    PriceKey key{entry->item_template, faction};

    std::shared_lock readLock(_pricesLock);
    auto it = _cachedPrices.find(key);
    readLock.unlock();

    if (it != _cachedPrices.end() && it->second.hasData)
    {
        std::unique_lock writeLock(_pricesLock);
        MarketPriceData& data = it->second;
        if (data.volume > 0)
            --data.volume;
        data.lastUpdate = time(nullptr);
    }
}

MarketPriceData MarketAnalyzer::GetMarketPrice(uint32 itemEntry, AuctionHouseFaction faction) const
{
    EnsureLoaded();

    PriceKey key{itemEntry, faction};
    std::shared_lock lock(_pricesLock);

    auto it = _cachedPrices.find(key);
    if (it != _cachedPrices.end())
        return it->second;

    return MarketPriceData{};
}

float MarketAnalyzer::GetPriceTrend(uint32 itemEntry, AuctionHouseFaction faction, uint32 days) const
{
    // Simplified - return 0 for now to avoid complex query issues
    return 0.0f;
}

uint64 MarketAnalyzer::GetMarketValue(uint32 itemEntry, AuctionHouseFaction faction) const
{
    MarketPriceData data = GetMarketPrice(itemEntry, faction);
    if (!data.hasData)
        return 0;

    return data.medianBuyout > 0 ? data.medianBuyout : data.avgBuyout;
}

void MarketAnalyzer::DailySnapshot()
{
    std::shared_lock lock(_pricesLock);
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    char dateStr[11];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localTime);

    for (const auto& [key, data] : _cachedPrices)
    {
        if (!data.hasData)
            continue;

        SavePriceSnapshot(key.itemEntry, key.faction, data, dateStr);
    }

    CleanupOldHistory(sAuctionHouseConfig.GetPriceHistoryDays());
    LOG_INFO("modules.auctionhouse", "Daily price snapshot completed");
}

void MarketAnalyzer::CleanupOldHistory(uint32 daysToKeep)
{
    CharacterDatabase.Execute(Acore::StringFormat(
        "DELETE FROM auctionhouse_price_history WHERE snapshot_date < DATE_SUB(CURDATE(), INTERVAL {} DAY)", daysToKeep));
}

void MarketAnalyzer::SavePriceSnapshot(uint32 itemEntry, AuctionHouseFaction faction, const MarketPriceData& data, const char* dateStr)
{
    CharacterDatabase.Execute(Acore::StringFormat(
        "INSERT INTO auctionhouse_price_history (item_entry, faction, min_buyout, avg_buyout, median_buyout, volume, snapshot_date) "
        "VALUES ({}, {}, {}, {}, {}, {}, '{}') "
        "ON DUPLICATE KEY UPDATE min_buyout=VALUES(min_buyout), avg_buyout=VALUES(avg_buyout), median_buyout=VALUES(median_buyout), volume=VALUES(volume)",
        itemEntry, static_cast<uint8>(faction), data.minBuyout, data.avgBuyout, data.medianBuyout, data.volume, dateStr));
}

void MarketAnalyzer::CalculateMedian(std::vector<uint64> buyouts, uint64& median) const
{
    if (buyouts.empty())
    {
        median = 0;
        return;
    }

    std::sort(buyouts.begin(), buyouts.end());
    size_t size = buyouts.size();

    if (size % 2 == 0)
        median = (buyouts[size / 2 - 1] + buyouts[size / 2]) / 2;
    else
        median = buyouts[size / 2];
}

AuctionHouseFaction MarketAnalyzer::GetFactionFromAH(AuctionHouseObject const* ah)
{
    AuctionHouseObject const* allianceAH = sAuctionMgr->GetAuctionsMapByHouseId(AuctionHouseId::Alliance);
    AuctionHouseObject const* hordeAH = sAuctionMgr->GetAuctionsMapByHouseId(AuctionHouseId::Horde);

    if (ah == allianceAH)
        return AuctionHouseFaction::Alliance;
    if (ah == hordeAH)
        return AuctionHouseFaction::Horde;
    return AuctionHouseFaction::Neutral;
}