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

#include "BlackMarketItemPool.h"
#include "BlackMarketAuctionHouse.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "WorldDatabase.h"

BlackMarketItemPool::BlackMarketItemPool() : _rng(std::random_device{}())
{
}

void BlackMarketItemPool::LoadFromDB()
{
    // Load from blackmarket_item_pools table
    // If table is empty, fall back to config
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_BMAH_ITEM_POOLS);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            std::string category = fields[1].Get<std::string>();
            uint32 itemEntry = fields[2].Get<uint32>();
            uint32 weight = fields[3].Get<uint32>();
            uint32 minIlvl = fields[4].Get<uint32>();
            uint32 maxIlvl = fields[5].Get<uint32>();
            uint64 minBid = fields[6].Get<uint64>();
            bool enabled = fields[7].Get<bool>();

            PoolItem item;
            item.itemEntry = itemEntry;
            item.weight = weight;
            item.minIlvl = minIlvl;
            item.maxIlvl = maxIlvl;
            item.minBid = minBid;
            item.enabled = enabled;

            _pools[category].push_back(item);
        } while (result->NextRow());

        LOG_INFO("modules.auctionhouse", "BMAH Item Pool loaded from DB: {} categories", _pools.size());
        return;
    }

    // Fallback to config
    LoadFromConfig();
}

void BlackMarketItemPool::LoadFromConfig()
{
    // Parse config strings for each pool category
    struct PoolConfig
    {
        const char* configKey;
        const char* categoryName;
    };

    PoolConfig configs[] = {
        {"BlackMarketAH.Pool.Mounts", "Mounts"},
        {"BlackMarketAH.Pool.Pets", "Pets"},
        {"BlackMarketAH.Pool.Transmog", "Transmog"},
        {"BlackMarketAH.Pool.Gear", "Gear"},
        {"BlackMarketAH.Pool.TCG", "TCG"},
        {"BlackMarketAH.Pool.Misc", "Misc"}
    };

    for (const auto& cfg : configs)
    {
        std::string configStr = sConfigMgr->GetOption<std::string>(cfg.configKey, "");
        if (configStr.empty())
            continue;

        // Parse format: "weight:minIlvl:maxIlvl:entry1,entry2,..."
        size_t pos1 = configStr.find(':');
        size_t pos2 = configStr.find(':', pos1 + 1);
        size_t pos3 = configStr.find(':', pos2 + 1);

        if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos)
            continue;

        uint32 weight = std::stoul(configStr.substr(0, pos1));
        uint32 minIlvl = std::stoul(configStr.substr(pos1 + 1, pos2 - pos1 - 1));
        uint32 maxIlvl = std::stoul(configStr.substr(pos2 + 1, pos3 - pos2 - 1));
        std::string entriesStr = configStr.substr(pos3 + 1);

        std::vector<uint32> entries;
        size_t start = 0;
        while (true)
        {
            size_t comma = entriesStr.find(',', start);
            if (comma == std::string::npos)
            {
                entries.push_back(std::stoul(entriesStr.substr(start)));
                break;
            }
            entries.push_back(std::stoul(entriesStr.substr(start, comma - start)));
            start = comma + 1;
        }

        for (uint32 entry : entries)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(entry);
            if (!proto)
                continue;

            if (proto->ItemLevel < minIlvl || proto->ItemLevel > maxIlvl)
                continue;

            PoolItem item;
            item.itemEntry = entry;
            item.weight = weight;
            item.minIlvl = minIlvl;
            item.maxIlvl = maxIlvl;
            item.minBid = 0;
            item.enabled = true;

            _pools[cfg.categoryName].push_back(item);
        }

        LOG_INFO("modules.auctionhouse", "BMAH Pool '{}': {} items loaded", cfg.categoryName, _pools[cfg.categoryName].size());
    }
}

std::vector<BMAHAuctionEntry> BlackMarketItemPool::SelectItemsForRotation(uint32 maxItems, uint32 rotationId)
{
    std::vector<BMAHAuctionEntry> selected;

    // Collect all enabled items with weights
    struct WeightedItem
    {
        PoolItem item;
        std::string category;
    };

    std::vector<WeightedItem> allItems;
    uint32 totalWeight = 0;

    for (auto& [category, items] : _pools)
    {
        for (auto& item : items)
        {
            if (!item.enabled)
                continue;

            allItems.push_back({item, category});
            totalWeight += item.weight;
        }
    }

    if (allItems.empty() || totalWeight == 0)
        return selected;

    // Weighted random selection
    std::uniform_int_distribution<uint32> dist(1, totalWeight);

    while (selected.size() < maxItems && !allItems.empty())
    {
        uint32 roll = dist(_rng);
        uint32 cumulativeWeight = 0;

        for (size_t i = 0; i < allItems.size(); ++i)
        {
            cumulativeWeight += allItems[i].item.weight;
            if (roll <= cumulativeWeight)
            {
                // Select this item
                BMAHAuctionEntry entry;
                entry.itemEntry = allItems[i].item.itemEntry;
                entry.itemCount = 1;
                entry.startBid = CalculateStartBid(allItems[i].item);
                entry.poolCategory = allItems[i].category;
                entry.rotationId = rotationId;

                selected.push_back(entry);

                // Remove from pool to avoid duplicates
                allItems.erase(allItems.begin() + i);
                totalWeight -= allItems[i].item.weight;
                break;
            }
        }
    }

    LOG_INFO("modules.auctionhouse", "BMAH Rotation {}: Selected {} items", rotationId, selected.size());
    return selected;
}

uint64 BlackMarketItemPool::CalculateStartBid(const PoolItem& item) const
{
    // Base bid on item quality, level, and minBid config
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemEntry);
    if (!proto)
        return item.minBid;

    uint64 baseBid = item.minBid;

    // Scale by item level
    if (proto->ItemLevel > 0)
    {
        baseBid += static_cast<uint64>(proto->ItemLevel) * 10000; // 1g per ilvl
    }

    // Scale by quality
    switch (proto->Quality)
    {
        case ITEM_QUALITY_POOR:      baseBid = baseBid * 50 / 100; break;
        case ITEM_QUALITY_NORMAL:    baseBid = baseBid * 75 / 100; break;
        case ITEM_QUALITY_UNCOMMON:  baseBid = baseBid * 100 / 100; break;
        case ITEM_QUALITY_RARE:      baseBid = baseBid * 200 / 100; break;
        case ITEM_QUALITY_EPIC:      baseBid = baseBid * 500 / 100; break;
        case ITEM_QUALITY_LEGENDARY: baseBid = baseBid * 2000 / 100; break;
        case ITEM_QUALITY_ARTIFACT:  baseBid = baseBid * 5000 / 100; break;
        case ITEM_QUALITY_HEIRLOOM:  baseBid = baseBid * 300 / 100; break;
        default: break;
    }

    return std::max<uint64>(baseBid, 10000); // Minimum 1g
}

uint32 BlackMarketItemPool::SelectRandomDuration() const
{
    uint32 minDur = sAuctionHouseConfig.GetBMAHMinDuration();
    uint32 maxDur = sAuctionHouseConfig.GetBMAHMaxDuration();
    return minDur + (_rng() % (maxDur - minDur + 1));
}