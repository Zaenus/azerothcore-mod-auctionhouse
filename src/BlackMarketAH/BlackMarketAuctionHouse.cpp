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

#include "BlackMarketAuctionHouse.h"
#include "BlackMarketMgr.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "GameTime.h"

BlackMarketAuctionHouse::BlackMarketAuctionHouse()
{
}

BlackMarketAuctionHouse::~BlackMarketAuctionHouse()
{
    for (auto& [id, auction] : _auctions)
    {
        // Clean up items if needed
    }
}

void BlackMarketAuctionHouse::AddAuction(const BMAHAuctionEntry& auctionData)
{
    std::lock_guard lock(_mutex);

    BMAHAuctionEntry entry = auctionData;
    entry.id = _nextAuctionId++;

    // Set expire time based on duration config
    uint32 minDur = sAuctionHouseConfig.GetBMAHMinDuration();
    uint32 maxDur = sAuctionHouseConfig.GetBMAHMaxDuration();
    uint32 duration = minDur + (rand() % (maxDur - minDur + 1));

    entry.expireTime = GameTime::GetGameTime().count() + duration;
    entry.currentBid = entry.startBid;

    _auctions[entry.id] = entry;

    LOG_DEBUG("modules.auctionhouse", "BMAH: Added auction #{} for item {} (bid: {}, expires in {}s)",
        entry.id, entry.itemEntry, entry.startBid, duration);
}

bool BlackMarketAuctionHouse::RemoveAuction(uint32 auctionId)
{
    std::lock_guard lock(_mutex);
    return _auctions.erase(auctionId) > 0;
}

BMAHAuctionEntry* BlackMarketAuctionHouse::GetAuction(uint32 auctionId)
{
    std::lock_guard lock(_mutex);
    auto it = _auctions.find(auctionId);
    return it != _auctions.end() ? &it->second : nullptr;
}

void BlackMarketAuctionHouse::Update()
{
    std::lock_guard lock(_mutex);
    time_t now = GameTime::GetGameTime().count();

    for (auto it = _auctions.begin(); it != _auctions.end(); )
    {
        if (it->second.expireTime <= now)
        {
            // Auction expired
            sBlackMarketMgr.OnAuctionExpired(it->first);
            it = _auctions.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void BlackMarketAuctionHouse::ClearExpiredAuctions()
{
    Update(); // Same logic
}