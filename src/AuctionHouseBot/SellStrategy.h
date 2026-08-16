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

#ifndef MOD_AUCTIONHOUSE_SELL_STRATEGY_H_
#define MOD_AUCTIONHOUSE_SELL_STRATEGY_H_

#include "Common.h"
#include "AuctionHouseMgr.h"
#include "MarketAnalyzer.h"
#include <vector>

class AuctionHouseBot;

struct SellCandidate
{
    uint32 itemEntry = 0;
    uint32 itemCount = 0;
    uint64 marketValue = 0;
    uint64 minSellPrice = 0;
    uint64 deposit = 0;
    uint32 duration = 0; // in minutes (12, 24, 48)
};

class SellStrategy
{
public:
    explicit SellStrategy(AuctionHouseBot* bot);
    ~SellStrategy() = default;

    void Execute();

private:
    void ScanInventory(std::vector<SellCandidate>& candidates);
    bool EvaluateItem(uint32 itemEntry, uint32 count, SellCandidate& candidate);
    bool ListItem(const SellCandidate& candidate);

    AuctionHouseBot* _bot = nullptr;
};

#endif