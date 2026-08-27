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

#ifndef MOD_AUCTIONHOUSE_RESTOCK_STRATEGY_H_
#define MOD_AUCTIONHOUSE_RESTOCK_STRATEGY_H_

#include "Common.h"
#include <vector>

class AuctionHouseBot;

struct RestockEntry
{
    uint32 itemEntry = 0;
    uint32 maxStack = 1;
};

class RestockStrategy
{
public:
    explicit RestockStrategy(AuctionHouseBot* bot);
    ~RestockStrategy() = default;

    void Execute();

    // Shared pool of eligible item entries, built once and reused by all bots
    static std::vector<RestockEntry> const& GetEligiblePool();
    static void InvalidatePool();

private:
    uint32 CountUnlistedStock() const;
    static RestockEntry PickWeightedEntry();

    AuctionHouseBot* _bot = nullptr;
};

#endif
