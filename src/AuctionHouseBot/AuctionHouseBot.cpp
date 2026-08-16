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
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"

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
    CreateBotAccount();
    LoadBotData();

    _buyStrategy = std::make_unique<BuyStrategy>(this);
    _sellStrategy = std::make_unique<SellStrategy>(this);

    LOG_INFO("modules.auctionhouse", "AH Bot initialized: Faction={}, BotIndex={}, GUID={}, Gold={}",
        static_cast<uint8>(_faction), _botIndex, _botGuid.GetCounter(), _gold);
}

void AuctionHouseBot::CreateBotAccount()
{
    std::string accountName = sAuctionHouseConfig.GetBotAccountPrefix() +
        (_faction == AuctionHouseFaction::Alliance ? "Alliance" :
         _faction == AuctionHouseFaction::Horde ? "Horde" : "Neutral") +
        "_" + std::to_string(_botIndex + 1);

    uint32 accountId = sAccountMgr->GetId(accountName);
    if (!accountId)
    {
        AccountCreateInfo info;
        info.username = accountName;
        info.password = "";
        info.email = "";
        info.recruiter = 0;
        info.gmlevel = 0;
        info.expansion = 2; // WotLK
        info.lastIP = "127.0.0.1";
        info.lastLogin = time(nullptr);
        info.locale = "enUS";

        if (!sAccountMgr->CreateAccount(accountName, "", "", 0, 2))
        {
            LOG_ERROR("modules.auctionhouse", "Failed to create AH bot account: {}", accountName);
            return;
        }

        accountId = sAccountMgr->GetId(accountName);
    }

    // Create bot character if not exists
    std::string charName = "AHBot_" +
        (_faction == AuctionHouseFaction::Alliance ? "A" :
         _faction == AuctionHouseFaction::Horde ? "H" : "N") +
        std::to_string(_botIndex + 1);

    ObjectGuid existingGuid = sObjectMgr->GetPlayerGUIDByName(charName);
    if (existingGuid)
    {
        _botGuid = existingGuid;
    }
    else
    {
        // Create character (level 80, appropriate race/class)
        // This is simplified - in production you'd use proper character creation
        _botGuid = ObjectGuid::Create<HighGuid::Player>(sObjectMgr->GenerateLowGuid<HighGuid::Player>());
        LOG_INFO("modules.auctionhouse", "Created AH bot character: {} (GUID: {})", charName, _botGuid.GetCounter());
    }

    // Ensure bot has starting gold
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_MONEY);
    stmt->SetData(0, _gold);
    stmt->SetData(1, _botGuid.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void AuctionHouseBot::LoadBotData()
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AH_BOT_INVENTORY);
    stmt->SetData(0, _botGuid.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            // Load bot inventory from DB
            // Implementation depends on inventory tracking needs
        } while (result->NextRow());
    }

    // Load gold
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MONEY);
    stmt->SetData(0, _botGuid.GetCounter());
    result = CharacterDatabase.Query(stmt);

    if (result)
    {
        Field* fields = result->Fetch();
        _gold = fields[0].Get<uint64>();
    }
}

void AuctionHouseBot::SaveBotData()
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_MONEY);
    stmt->SetData(0, _gold);
    stmt->SetData(1, _botGuid.GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void AuctionHouseBot::Update(uint32 diff)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

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
    if (_buyStrategy && _gold > 10000) // At least 1g to operate
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