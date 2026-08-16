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

#include "AuctionHouseConfig.h"
#include "Config.h"

void AuctionHouseConfig::Initialize(bool /*reload*/)
{
    LoadAHBotConfig();
    LoadBMAHConfig();
}

void AuctionHouseConfig::LoadAHBotConfig()
{
    _ahBotEnabled = sConfigMgr->GetOption<bool>("AuctionHouseBot.Enabled", true);
    _updateInterval = sConfigMgr->GetOption<uint32>("AuctionHouseBot.UpdateInterval", 300000);
    _maxActiveAuctionsPerBot = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxActiveAuctionsPerBot", 50);
    _maxGoldPerBot = sConfigMgr->GetOption<uint64>("AuctionHouseBot.MaxGoldPerBot", 10000000);
    _startingGoldPerBot = sConfigMgr->GetOption<uint64>("AuctionHouseBot.StartingGoldPerBot", 1000000);
    _minProfitMargin = sConfigMgr->GetOption<float>("AuctionHouseBot.MinProfitMargin", 0.15f);
    _maxBuyPricePercent = sConfigMgr->GetOption<float>("AuctionHouseBot.MaxBuyPricePercent", 0.80f);
    _minSellPricePercent = sConfigMgr->GetOption<float>("AuctionHouseBot.MinSellPricePercent", 1.10f);
    _priceHistoryDays = sConfigMgr->GetOption<uint32>("AuctionHouseBot.PriceHistoryDays", 30);
    _minItemLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MinItemLevel", 1);
    _maxItemLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxItemLevel", 277);

    std::string allowedClasses = sConfigMgr->GetOption<std::string>("AuctionHouseBot.AllowedItemClasses", "2,4,7,11");
    _allowedItemClasses = ParseCSVUInt32(allowedClasses);

    std::string blacklisted = sConfigMgr->GetOption<std::string>("AuctionHouseBot.BlacklistedItems", "");
    _blacklistedItems = ParseCSVUInt32(blacklisted);

    _botAccountPrefix = sConfigMgr->GetOption<std::string>("AuctionHouseBot.BotAccountPrefix", "AHBot_");
    _botCountPerFaction = sConfigMgr->GetOption<uint32>("AuctionHouseBot.BotCountPerFaction", 1);
    _maxItemsPerCycle = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxItemsPerCycle", 20);
    _relistThresholdHours = sConfigMgr->GetOption<uint32>("AuctionHouseBot.RelistThresholdHours", 12);
    _underpriceThreshold = sConfigMgr->GetOption<float>("AuctionHouseBot.UnderpriceThreshold", 0.50f);
}

void AuctionHouseConfig::LoadBMAHConfig()
{
    _bmahEnabled = sConfigMgr->GetOption<bool>("BlackMarketAH.Enabled", true);
    _bmahNPCEntry = sConfigMgr->GetOption<uint32>("BlackMarketAH.NPCEntry", 67766);
    _bmahRefreshDay = sConfigMgr->GetOption<uint8>("BlackMarketAH.RefreshDay", 3);
    _bmahRefreshHour = sConfigMgr->GetOption<uint32>("BlackMarketAH.RefreshHour", 3);
    _bmahMaxActiveAuctions = sConfigMgr->GetOption<uint32>("BlackMarketAH.MaxActiveAuctions", 10);
    _bmahMinDuration = sConfigMgr->GetOption<uint32>("BlackMarketAH.MinDuration", 86400);
    _bmahMaxDuration = sConfigMgr->GetOption<uint32>("BlackMarketAH.MaxDuration", 172800);
    _bmahDepositMultiplier = sConfigMgr->GetOption<float>("BlackMarketAH.DepositMultiplier", 0.0f);
    _bmahCutPercent = sConfigMgr->GetOption<uint32>("BlackMarketAH.CutPercent", 0);
    _bmahBidIncrementPercent = sConfigMgr->GetOption<float>("BlackMarketAH.BidIncrementPercent", 0.05f);

    _poolConfigs["Mounts"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.Mounts", ""));
    _poolConfigs["Pets"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.Pets", ""));
    _poolConfigs["Transmog"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.Transmog", ""));
    _poolConfigs["Gear"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.Gear", ""));
    _poolConfigs["TCG"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.TCG", ""));
    _poolConfigs["Misc"] = ParsePoolConfig(sConfigMgr->GetOption<std::string>("BlackMarketAH.Pool.Misc", ""));
}

std::set<uint32> AuctionHouseConfig::ParseCSVUInt32(const std::string& str) const
{
    std::set<uint32> result;
    if (str.empty())
        return result;

    size_t start = 0;
    while (start < str.size())
    {
        size_t comma = str.find(',', start);
        if (comma == std::string::npos)
            comma = str.size();

        std::string token = str.substr(start, comma - start);
        if (!token.empty())
        {
            try
            {
                result.insert(static_cast<uint32>(std::stoul(token)));
            }
            catch (...)
            {
                // Ignore invalid entries
            }
        }

        start = comma + 1;
    }
    return result;
}

BMAHPoolConfig AuctionHouseConfig::ParsePoolConfig(const std::string& str) const
{
    BMAHPoolConfig config;
    if (str.empty())
        return config;

    // Format: "weight:minIlvl:maxIlvl:entry1,entry2,..."
    size_t pos1 = str.find(':');
    size_t pos2 = str.find(':', pos1 + 1);
    size_t pos3 = str.find(':', pos2 + 1);

    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos)
        return config;

    config.weight = std::stoul(str.substr(0, pos1));
    config.minIlvl = std::stoul(str.substr(pos1 + 1, pos2 - pos1 - 1));
    config.maxIlvl = std::stoul(str.substr(pos2 + 1, pos3 - pos2 - 1));

    std::string entriesStr = str.substr(pos3 + 1);
    size_t start = 0;
    while (start < entriesStr.size())
    {
        size_t comma = entriesStr.find(',', start);
        if (comma == std::string::npos)
            comma = entriesStr.size();

        std::string token = entriesStr.substr(start, comma - start);
        if (!token.empty())
        {
            try
            {
                config.itemEntries.push_back(std::stoul(token));
            }
            catch (...)
            {
                // Ignore invalid entries
            }
        }

        start = comma + 1;
    }

    return config;
}

const BMAHPoolConfig& AuctionHouseConfig::GetPoolConfig(const std::string& category) const
{
    static BMAHPoolConfig emptyConfig;
    auto it = _poolConfigs.find(category);
    return it != _poolConfigs.end() ? it->second : emptyConfig;
}