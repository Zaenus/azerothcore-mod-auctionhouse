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

std::vector<RestockEntry> const& RestockStrategy::GetEligiblePool()
{
    static std::vector<RestockEntry> pool;
    static std::once_flag poolOnce;

    std::call_once(poolOnce, []()
    {
        auto const& allowedClasses = sAuctionHouseConfig.GetAllowedItemClasses();
        auto const& blacklisted = sAuctionHouseConfig.GetBlacklistedItems();

        uint32 maxQuality = sAuctionHouseConfig.GetRestockMaxQuality();

        for (auto const& [entry, proto] : *sObjectMgr->GetItemTemplateStore())
        {
            // Skip blacklisted items
            if (blacklisted.count(entry))
                continue;

            // Respect configured item classes
            if (!allowedClasses.empty() && !allowedClasses.count(proto.Class))
                continue;

            // Respect configured item level range
            if (proto.ItemLevel < sAuctionHouseConfig.GetMinItemLevel() ||
                proto.ItemLevel > sAuctionHouseConfig.GetMaxItemLevel())
                continue;

            // Quality cap (0=Poor .. 3=Rare by default) to keep the market believable
            if (proto.Quality > maxQuality)
                continue;

            // Only unbound items can realistically appear on the AH in volume
            if (proto.Bonding != NO_BIND)
                continue;

            // No conjured goods
            if (proto.HasFlag(ITEM_FLAG_CONJURED))
                continue;

            // Must have a usable value estimate, otherwise SellStrategy would reject it anyway
            uint64 estimate = proto.BuyPrice > 0 ? static_cast<uint64>(proto.BuyPrice) : proto.SellPrice * 4ull;
            if (estimate == 0)
                continue;

            pool.push_back({ entry, proto.GetMaxStackSize() });
        }

        LOG_INFO("modules.auctionhouse", "AH Bot restock pool built with {} eligible item entries", pool.size());
    });

    return pool;
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
        RestockEntry const& restockEntry = pool[urand(0, pool.size() - 1)];

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
