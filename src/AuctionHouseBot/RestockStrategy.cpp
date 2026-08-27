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

#include "RestockStrategy.h"
#include "AuctionHouseBot.h"
#include "Config/AuctionHouseConfig.h"
#include "CharacterDatabase.h"
#include "ItemTemplate.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "QueryResult.h"
#include "Random.h"
#include "Utilities/StringFormat.h"
#include <mutex>

namespace
{
    struct WeightedPools
    {
        std::vector<RestockEntry> byClass[17];
        std::vector<RestockEntry> epicPool;
        std::vector<RestockEntry> normalPool;
        size_t totalNormal = 0;
    };

    WeightedPools g_pools;
    std::once_flag g_poolOnce;
    std::mutex g_poolMutex;
    bool g_poolBuilt = false;

    void BuildPools()
    {
        auto const& allowedClasses = sAuctionHouseConfig.GetAllowedItemClasses();
        auto const& blacklisted = sAuctionHouseConfig.GetBlacklistedItems();
        uint32 maxQuality = sAuctionHouseConfig.GetRestockMaxQuality();
        bool allowBoE = sAuctionHouseConfig.GetRestockAllowBoE();

        for (auto const& [entry, proto] : *sObjectMgr->GetItemTemplateStore())
        {
            if (blacklisted.count(entry))
                continue;
            if (!allowedClasses.empty() && !allowedClasses.count(proto.Class))
                continue;
            if (proto.ItemLevel < sAuctionHouseConfig.GetMinItemLevel() ||
                proto.ItemLevel > sAuctionHouseConfig.GetMaxItemLevel())
                continue;

            // Quality handling: normal up to maxQuality, epic (4) is gated by EpicChance
            bool isEpic = proto.Quality == 4; // ITEM_QUALITY_EPIC
            if (proto.Quality > maxQuality && !isEpic)
                continue;
            if (isEpic && proto.Quality > 4)
                continue;

            // Bonding: allow BoE if configured, otherwise only NO_BIND
            // Disallow BoP and Quest binds; allow NO_BIND, BIND_WHEN_EQUIPPED, BIND_WHEN_USE
            if (proto.Bonding == BIND_WHEN_PICKED_UP || proto.Bonding == BIND_QUEST_ITEM || proto.Bonding == BIND_QUEST_ITEM1)
                continue;
            if (!allowBoE && proto.Bonding != NO_BIND)
                continue;

            if (proto.HasFlag(ITEM_FLAG_CONJURED))
                continue;

            uint64 estimate = proto.BuyPrice > 0 ? static_cast<uint64>(proto.BuyPrice) : proto.SellPrice * 4ull;
            if (estimate == 0)
                continue;

            // Skip deprecated / test items with empty names or zero display
            if (proto.Class >= 17)
                continue;

            RestockEntry e{ entry, proto.GetMaxStackSize() };
            if (proto.Class < 17)
                g_pools.byClass[proto.Class].push_back(e);

            if (isEpic)
                g_pools.epicPool.push_back(e);
            else
                g_pools.normalPool.push_back(e);
        }

        g_pools.totalNormal = g_pools.normalPool.size();
        g_poolBuilt = true;
        LOG_INFO("modules.auctionhouse", "AH Bot restock pools built: normal={} epic={} totalByClass (0:{},1:{},2:{},3:{},4:{},5:{},7:{},9:{},15:{},16:{})",
            g_pools.normalPool.size(), g_pools.epicPool.size(),
            g_pools.byClass[0].size(), g_pools.byClass[1].size(), g_pools.byClass[2].size(),
            g_pools.byClass[3].size(), g_pools.byClass[4].size(), g_pools.byClass[5].size(),
            g_pools.byClass[7].size(), g_pools.byClass[9].size(), g_pools.byClass[15].size(), g_pools.byClass[16].size());
    }
}

std::vector<RestockEntry> const& RestockStrategy::GetEligiblePool()
{
    std::call_once(g_poolOnce, BuildPools);
    // Return legacy combined pool for backwards compat (normal + epic if chance)
    static std::vector<RestockEntry> combined;
    static std::once_flag combinedOnce;
    std::call_once(combinedOnce, []()
    {
        combined.reserve(g_pools.normalPool.size() + g_pools.epicPool.size());
        combined.insert(combined.end(), g_pools.normalPool.begin(), g_pools.normalPool.end());
        combined.insert(combined.end(), g_pools.epicPool.begin(), g_pools.epicPool.end());
    });
    return combined;
}

void RestockStrategy::InvalidatePool()
{
    std::lock_guard<std::mutex> lock(g_poolMutex);
    if (g_poolBuilt)
    {
        for (auto& v : g_pools.byClass)
            v.clear();
        g_pools.epicPool.clear();
        g_pools.normalPool.clear();
        g_poolBuilt = false;
        BuildPools();
    }
}

RestockEntry RestockStrategy::PickWeightedEntry()
{
    std::call_once(g_poolOnce, BuildPools);

    float epicChance = sAuctionHouseConfig.GetRestockEpicChance();
    if (!g_pools.epicPool.empty() && epicChance > 0.0f && roll_chance_f(epicChance * 100.0f))
        return g_pools.epicPool[urand(0, static_cast<uint32>(g_pools.epicPool.size() - 1))];

    // Weighted by class: 0 Consumable 20, 1 Container 2, 2 Weapon 15, 3 Gem 10, 4 Armor 15, 5 Reagent 5, 7 TradeGoods 25, 9 Recipe 5, 15 Misc 5 (mounts/pets), 16 Glyph 10
    // Total 112 -> normalize via roll
    struct Weight { uint32 cls; uint32 w; };
    static const Weight weights[] = {
        {0, 20}, {1, 2}, {2, 15}, {3, 10}, {4, 15}, {5, 5}, {7, 25}, {9, 5}, {15, 5}, {16, 10}
    };
    uint32 totalW = 0;
    for (auto const& wt : weights)
        if (!g_pools.byClass[wt.cls].empty())
            totalW += wt.w;

    if (totalW == 0)
    {
        // Fallback to normal pool uniform
        if (!g_pools.normalPool.empty())
            return g_pools.normalPool[urand(0, static_cast<uint32>(g_pools.normalPool.size() - 1))];
        return {0, 1};
    }

    uint32 roll = urand(1, totalW);
    uint32 acc = 0;
    for (auto const& wt : weights)
    {
        auto const& vec = g_pools.byClass[wt.cls];
        if (vec.empty())
            continue;
        acc += wt.w;
        if (roll <= acc)
            return vec[urand(0, static_cast<uint32>(vec.size() - 1))];
    }

    // Fallback
    return g_pools.normalPool[urand(0, static_cast<uint32>(g_pools.normalPool.size() - 1))];
}

RestockStrategy::RestockStrategy(AuctionHouseBot* bot) : _bot(bot)
{
}

uint32 RestockStrategy::CountUnlistedStock() const
{
    QueryResult result = CharacterDatabase.Query(Acore::StringFormat(
        "SELECT COUNT(*) FROM auctionhouse_bot_inventory WHERE bot_guid = {} AND listed = 0",
        _bot->GetBotGuid().GetCounter()));

    return result ? result->Fetch()[0].Get<uint32>() : 0;
}

void RestockStrategy::Execute()
{
    if (!sAuctionHouseConfig.IsRestockEnabled())
        return;

    auto const& pool = GetEligiblePool();
    if (pool.empty())
        return;

    uint32 stock = CountUnlistedStock();
    uint32 target = sAuctionHouseConfig.GetRestockMinStockPerBot();
    if (stock >= target)
        return;

    uint32 toAdd = std::min(target - stock, sAuctionHouseConfig.GetRestockMaxPerCycle());
    uint32 added = 0;

    for (uint32 i = 0; i < toAdd; ++i)
    {
        RestockEntry restockEntry = PickWeightedEntry();
        if (restockEntry.itemEntry == 0)
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(restockEntry.itemEntry);
        if (!proto)
            continue;

        // Randomized stack size, capped so deposits stay sane
        uint32 maxStack = std::max<uint32>(1u, std::min<uint32>(restockEntry.maxStack, 20u));
        uint32 count = maxStack > 1 ? urand(1, maxStack) : 1;

        // Track an approximate acquisition cost for bookkeeping
        uint64 acquiredPrice = proto->BuyPrice > 0
            ? static_cast<uint64>(proto->BuyPrice) * count
            : proto->SellPrice * 4ull * count;

        CharacterDatabase.Execute(Acore::StringFormat(
            "INSERT INTO auctionhouse_bot_inventory (bot_guid, item_guid, item_entry, count, acquired_price, acquired_date, listed) "
            "VALUES ({}, {}, {}, {}, {}, CURDATE(), 0)",
            _bot->GetBotGuid().GetCounter(),
            sObjectMgr->GetGenerator<HighGuid::Item>().Generate(),
            restockEntry.itemEntry, count, acquiredPrice));

        ++added;
    }

    if (added > 0)
    {
        LOG_INFO("modules.auctionhouse", "AH Bot (Faction={}, Index={}) restocked {} items (stock was {}/{})",
            static_cast<uint8>(_bot->GetFaction()), _bot->GetBotIndex(), added, stock, target);
    }
}
