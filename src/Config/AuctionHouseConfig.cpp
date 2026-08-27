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
#include "AuctionHouseBot/RestockStrategy.h"

void AuctionHouseConfig::Initialize(bool reload)
{
    LoadAHBotConfig();
    if (reload)
        RestockStrategy::InvalidatePool();
}

void AuctionHouseConfig::LoadAHBotConfig()
{
    _ahBotEnabled = sConfigMgr->GetOption<bool>("AuctionHouseBot.Enabled", true);
    _updateInterval = sConfigMgr->GetOption<uint32>("AuctionHouseBot.UpdateInterval", 300000);
    _maxActiveAuctionsPerBot = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxActiveAuctionsPerBot", 150);
    _maxGoldPerBot = sConfigMgr->GetOption<uint64>("AuctionHouseBot.MaxGoldPerBot", 10000000);
    _startingGoldPerBot = sConfigMgr->GetOption<uint64>("AuctionHouseBot.StartingGoldPerBot", 1000000);
    _minProfitMargin = sConfigMgr->GetOption<float>("AuctionHouseBot.MinProfitMargin", 0.15f);
    _maxBuyPricePercent = sConfigMgr->GetOption<float>("AuctionHouseBot.MaxBuyPricePercent", 0.80f);
    _minSellPricePercent = sConfigMgr->GetOption<float>("AuctionHouseBot.MinSellPricePercent", 1.10f);
    _priceHistoryDays = sConfigMgr->GetOption<uint32>("AuctionHouseBot.PriceHistoryDays", 30);
    _minItemLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MinItemLevel", 1);
    _maxItemLevel = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxItemLevel", 277);

    std::string allowedClasses = sConfigMgr->GetOption<std::string>("AuctionHouseBot.AllowedItemClasses", "0,1,2,3,4,5,7,9,15,16");
    _allowedItemClasses = ParseCSVUInt32(allowedClasses);

    std::string blacklisted = sConfigMgr->GetOption<std::string>("AuctionHouseBot.BlacklistedItems", "");
    _blacklistedItems = ParseCSVUInt32(blacklisted);

    _botAccountPrefix = sConfigMgr->GetOption<std::string>("AuctionHouseBot.BotAccountPrefix", "AHBot_");
    _botCountPerFaction = sConfigMgr->GetOption<uint32>("AuctionHouseBot.BotCountPerFaction", 1);
    _maxItemsPerCycle = sConfigMgr->GetOption<uint32>("AuctionHouseBot.MaxItemsPerCycle", 20);
    _relistThresholdHours = sConfigMgr->GetOption<uint32>("AuctionHouseBot.RelistThresholdHours", 12);
    _underpriceThreshold = sConfigMgr->GetOption<float>("AuctionHouseBot.UnderpriceThreshold", 0.50f);

    _restockEnabled = sConfigMgr->GetOption<bool>("AuctionHouseBot.Restock.Enabled", true);
    _restockMinStockPerBot = sConfigMgr->GetOption<uint32>("AuctionHouseBot.Restock.MinStockPerBot", 40);
    _restockMaxPerCycle = sConfigMgr->GetOption<uint32>("AuctionHouseBot.Restock.MaxPerCycle", 15);
    _restockMaxQuality = sConfigMgr->GetOption<uint32>("AuctionHouseBot.Restock.MaxQuality", 3);
    _priceVariancePercent = sConfigMgr->GetOption<float>("AuctionHouseBot.PriceVariancePercent", 15.0f);
    _refillGoldBelow = sConfigMgr->GetOption<uint64>("AuctionHouseBot.RefillGoldBelow", 500000);
    _restockAllowBoE = sConfigMgr->GetOption<bool>("AuctionHouseBot.Restock.AllowBoE", true);
    _restockEpicChance = sConfigMgr->GetOption<float>("AuctionHouseBot.Restock.EpicChance", 0.05f);
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