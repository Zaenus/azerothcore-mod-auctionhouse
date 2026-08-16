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

#ifndef MOD_AUCTIONHOUSE_BLACKMARKET_MGR_H_
#define MOD_AUCTIONHOUSE_BLACKMARKET_MGR_H_

#include "Common.h"
#include "AuctionHouseMgr.h"
#include "Config/AuctionHouseConfig.h"
#include <mutex>
#include <vector>

class BlackMarketAuctionHouse;
class BlackMarketItemPool;
class BlackMarketRotation;
class BlackMarketBidManager;

struct BMAHAuction
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

class BlackMarketMgr
{
public:
    static BlackMarketMgr& Instance()
    {
        static BlackMarketMgr instance;
        return instance;
    }

    void Initialize();
    void Update(uint32 diff);
    void LoadFromDB();
    void SaveToDB();

    BlackMarketAuctionHouse* GetAuctionHouse() { return _auctionHouse.get(); }
    BlackMarketItemPool* GetItemPool() { return _itemPool.get(); }
    BlackMarketRotation* GetRotation() { return _rotation.get(); }
    BlackMarketBidManager* GetBidManager() { return _bidManager.get(); }

    bool IsEnabled() const { return _enabled; }
    uint32 GetNPCEntry() const { return _npcEntry; }

    void OnAuctionExpired(uint32 auctionId);
    void OnAuctionWon(uint32 auctionId, ObjectGuid winnerGuid, uint64 bidAmount);
    void OnBidPlaced(uint32 auctionId, ObjectGuid bidderGuid, uint64 bidAmount);

private:
    BlackMarketMgr() = default;
    ~BlackMarketMgr() = default;

    BlackMarketMgr(const BlackMarketMgr&) = delete;
    BlackMarketMgr& operator=(const BlackMarketMgr&) = delete;

    void CreateAuctionHouse();
    void SpawnRotationItems();

    bool _enabled = false;
    uint32 _npcEntry = 67766;
    std::unique_ptr<BlackMarketAuctionHouse> _auctionHouse;
    std::unique_ptr<BlackMarketItemPool> _itemPool;
    std::unique_ptr<BlackMarketRotation> _rotation;
    std::unique_ptr<BlackMarketBidManager> _bidManager;
    std::mutex _mutex;
};

#define sBlackMarketMgr BlackMarketMgr::Instance()

#endif