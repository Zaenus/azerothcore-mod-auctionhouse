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

#ifndef MOD_AUCTIONHOUSE_BOT_H_
#define MOD_AUCTIONHOUSE_BOT_H_

#include "Common.h"
#include "AuctionHouseMgr.h"
#include "MarketAnalyzer.h"
#include <memory>

class AuctionHouseBotMgr;
class BuyStrategy;
class SellStrategy;

class AuctionHouseBot
{
public:
    AuctionHouseBot(AuctionHouseBotMgr* mgr, AuctionHouseFaction faction, uint32 botIndex);
    ~AuctionHouseBot();

    void Initialize();
    void Update(uint32 diff);

    AuctionHouseFaction GetFaction() const { return _faction; }
    uint32 GetBotIndex() const { return _botIndex; }
    ObjectGuid GetBotGuid() const { return _botGuid; }
    uint64 GetGold() const { return _gold; }
    void AddGold(uint64 amount) { _gold += amount; }
    bool SpendGold(uint64 amount);

    uint32 GetActiveAuctionCount() const { return _activeAuctionCount; }
    void SetActiveAuctionCount(uint32 count) { _activeAuctionCount = count; }

private:
    void CreateBotAccount();
    void LoadBotData();
    void SaveBotData();
    void EnsureLoaded();

    AuctionHouseBotMgr* _mgr = nullptr;
    AuctionHouseFaction _faction;
    uint32 _botIndex = 0;
    ObjectGuid _botGuid;
    uint64 _gold = 0;
    uint32 _activeAuctionCount = 0;
    time_t _lastUpdate = 0;
    bool _loaded = false;
    mutable std::mutex _loadLock;

    std::unique_ptr<BuyStrategy> _buyStrategy;
    std::unique_ptr<SellStrategy> _sellStrategy;
};

#endif