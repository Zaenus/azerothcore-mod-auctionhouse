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

#ifndef MOD_AUCTIONHOUSE_BLACKMARKET_AUCTIONHOUSE_H_
#define MOD_AUCTIONHOUSE_BLACKMARKET_AUCTIONHOUSE_H_

#include "AuctionHouseMgr.h"
#include <map>
#include <mutex>

struct BMAHAuctionEntry
{
    uint32 id = 0;
    uint32 itemEntry = 0;
    ObjectGuid itemGuid;
    uint64 startBid = 0;
    uint64 currentBid = 0;
    ObjectGuid bidderGuid;
    time_t expireTime = 0;
    std::string poolCategory;
    uint32 rotationId = 0;
};

class BlackMarketAuctionHouse
{
public:
    BlackMarketAuctionHouse();
    ~BlackMarketAuctionHouse();

    void AddAuction(const BMAHAuctionEntry& auctionData);
    bool RemoveAuction(uint32 auctionId);
    BMAHAuctionEntry* GetAuction(uint32 auctionId);
    const std::map<uint32, BMAHAuctionEntry>& GetAuctions() const { return _auctions; }
    uint32 Getcount() const { return static_cast<uint32>(_auctions.size()); }

    void Update();
    void ClearExpiredAuctions();

    // BMAH-specific: no deposit, no cut, no buyout, bid-only
    static uint32 GetDeposit(uint32 /*itemEntry*/, uint32 /*duration*/) { return 0; }
    static uint32 GetCut(uint64 /*bid*/) { return 0; }
    static bool HasBuyout() { return false; }
    static float GetMinBidIncrement() { return sAuctionHouseConfig.GetBMAHBidIncrementPercent(); }

private:
    std::map<uint32, BMAHAuctionEntry> _auctions;
    uint32 _nextAuctionId = 1;
    std::mutex _mutex;
};

#endif