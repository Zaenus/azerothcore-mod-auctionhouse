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

#ifndef MOD_AUCTIONHOUSE_BLACKMARKET_ITEM_POOL_H_
#define MOD_AUCTIONHOUSE_BLACKMARKET_ITEM_POOL_H_

#include "Common.h"
#include "Config/AuctionHouseConfig.h"
#include "BlackMarketAuctionHouse.h"
#include <map>
#include <vector>
#include <random>

struct PoolItem
{
    uint32 itemEntry = 0;
    uint32 weight = 1;
    uint32 minIlvl = 1;
    uint32 maxIlvl = 277;
    uint64 minBid = 0;
    bool enabled = true;
};

class BlackMarketItemPool
{
public:
    BlackMarketItemPool();
    ~BlackMarketItemPool() = default;

    void LoadFromDB();
    void LoadFromConfig();

    std::vector<BMAHAuctionEntry> SelectItemsForRotation(uint32 maxItems, uint32 rotationId);

private:
    std::map<std::string, std::vector<PoolItem>> _pools;
    mutable std::mt19937 _rng;

    uint64 CalculateStartBid(const PoolItem& item) const;
    uint32 SelectRandomDuration() const;
};

#endif