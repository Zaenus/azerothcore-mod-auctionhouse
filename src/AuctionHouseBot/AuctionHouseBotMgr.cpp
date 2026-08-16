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

#include "AuctionHouseBotMgr.h"
#include "AuctionHouseBot.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"

void AuctionHouseBotMgr::Initialize()
{
    if (_initialized)
        return;

    std::lock_guard lock(_botsLock);

    if (!sAuctionHouseConfig.IsAHBotEnabled())
    {
        LOG_INFO("modules.auctionhouse", "AH Bot module is disabled");
        return;
    }

    CreateDefaultBots();
    _initialized = true;

    LOG_INFO("modules.auctionhouse", "AH Bot Manager initialized with {} bots per faction",
        sAuctionHouseConfig.GetBotCountPerFaction());
}

void AuctionHouseBotMgr::CreateDefaultBots()
{
    uint32 botCount = sAuctionHouseConfig.GetBotCountPerFaction();

    for (uint8 f = 0; f < 3; ++f)
    {
        AuctionHouseFaction faction = static_cast<AuctionHouseFaction>(f);
        _bots[f].reserve(botCount);

        for (uint32 i = 0; i < botCount; ++i)
        {
            auto bot = std::make_unique<AuctionHouseBot>(this, faction, i);
            bot->Initialize();
            _bots[f].push_back(std::move(bot));
        }
    }
}

void AuctionHouseBotMgr::Update(uint32 diff)
{
    if (!_initialized || !sAuctionHouseConfig.IsAHBotEnabled())
        return;

    std::lock_guard lock(_botsLock);

    for (auto& factionBots : _bots)
    {
        for (auto& bot : factionBots)
        {
            if (bot)
                bot->Update(diff);
        }
    }
}

AuctionHouseBot* AuctionHouseBotMgr::GetBot(AuctionHouseFaction faction, uint32 index)
{
    std::lock_guard lock(_botsLock);

    uint8 f = static_cast<uint8>(faction);
    if (f >= 3 || index >= _bots[f].size())
        return nullptr;

    return _bots[f][index].get();
}

size_t AuctionHouseBotMgr::GetBotCount(AuctionHouseFaction faction) const
{
    std::lock_guard lock(_botsLock);
    uint8 f = static_cast<uint8>(faction);
    if (f >= 3)
        return 0;
    return _bots[f].size();
}