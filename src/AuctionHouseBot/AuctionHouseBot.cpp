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

#include "AuctionHouseBot.h"
#include "AuctionHouseBotMgr.h"
#include "BuyStrategy.h"
#include "SellStrategy.h"
#include "Config/AuctionHouseConfig.h"
#include "AccountMgr.h"
#include "CharacterDatabase.h"
#include "Chat.h"
#include "QueryResult.h"
#include "Logging/Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Utilities/StringFormat.h"

AuctionHouseBot::AuctionHouseBot(AuctionHouseBotMgr* mgr, AuctionHouseFaction faction, uint32 botIndex)
    : _mgr(mgr), _faction(faction), _botIndex(botIndex)
{
}

AuctionHouseBot::~AuctionHouseBot()
{
    SaveBotData();
}

void AuctionHouseBot::Initialize()
{
    _gold = sAuctionHouseConfig.GetStartingGoldPerBot();

    _buyStrategy = std::make_unique<BuyStrategy>(this);
    _sellStrategy = std::make_unique<SellStrategy>(this);

    LOG_INFO("modules.auctionhouse", "AH Bot initialized: Faction={}, BotIndex={}, GUID={}, Gold={}",
        static_cast<uint8>(_faction), _botIndex, _botGuid.GetCounter(), _gold);
}

void AuctionHouseBot::EnsureLoaded()
{
    if (_loaded)
        return;

    std::lock_guard lock(_loadLock);
    if (_loaded)
        return;

    LoadBotData();
    _loaded = true;

    LOG_INFO("modules.auctionhouse", "AH Bot loaded: Faction={}, BotIndex={}, GUID={}, Gold={}",
        static_cast<uint8>(_faction), _botIndex, _botGuid.GetCounter(), _gold);
}

void AuctionHouseBot::LoadBotData()
{
    // Use fixed GUIDs for bots (created via SQL)
    static const uint32 botGuids[3] = { 1000000, 1000001, 1000002 }; // Alliance, Horde, Neutral
    _botGuid = ObjectGuid::Create<HighGuid::Player>(botGuids[static_cast<uint8>(_faction)] + _botIndex);

    // Load gold from database
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER);
    stmt->SetData(0, _botGuid.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        Field* fields = result->Fetch();
        // money is at index 8 in CHAR_SEL_CHARACTER
        _gold = fields[8].Get<uint64>();
    }
    else
    {
        // Character doesn't exist, set default gold and update DB
        _gold = sAuctionHouseConfig.GetStartingGoldPerBot();
        SaveBotData();
    }

    // Load inventory
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_INVENTORY);
    stmt->SetData(0, _botGuid.GetCounter());
    result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            // Process inventory items if needed
        } while (result->NextRow());
    }
}

void AuctionHouseBot::SaveBotData()
{
    // Persist the bot's gold. CHAR_UPD_CHARACTER has money at param index 6 and
    // its index ordering is fragile, so use a targeted update instead.
    CharacterDatabase.Execute(Acore::StringFormat(
        "UPDATE characters SET money = {} WHERE guid = {}", _gold, _botGuid.GetCounter()));
}

void AuctionHouseBot::Update(uint32 diff)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    EnsureLoaded();

    time_t now = time(nullptr);
    if (now - _lastUpdate < sAuctionHouseConfig.GetUpdateInterval() / 1000)
        return;

    _lastUpdate = now;

    // Refresh active auction count
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(
        _faction == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        _faction == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

    if (ah)
    {
        _activeAuctionCount = 0;
        for (auto const& [id, auction] : ah->GetAuctions())
        {
            if (auction->owner == _botGuid)
                ++_activeAuctionCount;
        }
    }

    // Execute buy/sell strategies
    if (_buyStrategy && _gold > 10000)
        _buyStrategy->Execute();

    if (_sellStrategy)
        _sellStrategy->Execute();

    SaveBotData();
}

bool AuctionHouseBot::SpendGold(uint64 amount)
{
    if (_gold >= amount)
    {
        _gold -= amount;
        return true;
    }
    return false;
}