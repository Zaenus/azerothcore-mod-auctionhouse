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

#ifndef MOD_AUCTIONHOUSE_BOT_MGR_H_
#define MOD_AUCTIONHOUSE_BOT_MGR_H_

#include "Common.h"
#include "AuctionHouseBot.h"
#include <array>
#include <memory>
#include <mutex>

class AuctionHouseBotMgr
{
public:
    static AuctionHouseBotMgr& Instance()
    {
        static AuctionHouseBotMgr instance;
        return instance;
    }

    void Initialize();
    void Update(uint32 diff);
    void LoadBots();
    void CreateDefaultBots();

    AuctionHouseBot* GetBot(AuctionHouseFaction faction, uint32 index = 0);
    size_t GetBotCount(AuctionHouseFaction faction) const;

private:
    AuctionHouseBotMgr() = default;
    ~AuctionHouseBotMgr() = default;

    AuctionHouseBotMgr(const AuctionHouseBotMgr&) = delete;
    AuctionHouseBotMgr& operator=(const AuctionHouseBotMgr&) = delete;

    std::array<std::vector<std::unique_ptr<AuctionHouseBot>>, 3> _bots;
    std::mutex _botsLock;
    bool _initialized = false;
};

#define sAuctionHouseBotMgr AuctionHouseBotMgr::Instance()

#endif