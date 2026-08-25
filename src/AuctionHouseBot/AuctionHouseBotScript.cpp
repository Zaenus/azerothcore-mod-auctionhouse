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

#include "AuctionHouseBotScript.h"
#include "AuctionHouseBotMgr.h"
#include "MarketAnalyzer.h"
#include "Config/AuctionHouseConfig.h"
#include "CharacterDatabase.h"
#include "Item.h"
#include "Logging/Log.h"
#include "Utilities/StringFormat.h"
#include <algorithm>

void AuctionHouseBotScript::OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    MarketAnalyzer::Instance().RecordListing(ah, entry);
}

void AuctionHouseBotScript::OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    // Could track removals for analytics
}

void AuctionHouseBotScript::OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    MarketAnalyzer::Instance().RecordSale(ah, entry);

    // Credit the sale proceeds to the bot's virtual wallet so it can keep
    // paying deposits on future listings
    if (AuctionHouseBot* bot = sAuctionHouseBotMgr.FindBotByGuid(entry->owner))
    {
        uint64 net = entry->bid > entry->GetAuctionCut() ? entry->bid - entry->GetAuctionCut() : 0;
        uint64 maxGold = sAuctionHouseConfig.GetMaxGoldPerBot();
        uint64 room = bot->GetGold() < maxGold ? maxGold - bot->GetGold() : 0;
        bot->AddGold(std::min(net, room));
    }
}

void AuctionHouseBotScript::OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    MarketAnalyzer::Instance().RecordExpiry(ah, entry);

    // Return expired goods to the bot's virtual inventory so they get relisted
    // instead of being lost (the expired-mail is suppressed for bot auctions)
    if (AuctionHouseBot* bot = sAuctionHouseBotMgr.FindBotByGuid(entry->owner))
    {
        CharacterDatabase.Execute(Acore::StringFormat(
            "INSERT INTO auctionhouse_bot_inventory (bot_guid, item_guid, item_entry, count, acquired_price, acquired_date, listed) "
            "VALUES ({}, {}, {}, {}, {}, CURDATE(), 0)",
            bot->GetBotGuid().GetCounter(), entry->item_guid.GetCounter(),
            entry->item_template, entry->itemCount, entry->buyout));
    }
}

void AuctionHouseBotScript::OnBeforeAuctionHouseMgrUpdate()
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    sAuctionHouseBotMgr.Update(60000); // Called every minute by AH mgr
}

void AuctionHouseBotScript::OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* auction, Player* /*owner*/, uint32& /*owner_accId*/, bool& /*sendNotification*/, bool& sendMail)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    // Bot auctions must not mail items back: bots are virtual characters and
    // the item is re-added to their inventory by OnAuctionExpire instead.
    // Free the virtual item here so it is not leaked once removed from the AH.
    if (!auction || !sAuctionHouseBotMgr.FindBotByGuid(auction->owner))
        return;

    sendMail = false;

    if (Item* item = sAuctionMgr->GetAItem(auction->item_guid))
    {
        sAuctionMgr->RemoveAItem(auction->item_guid);
        delete item;
        CharacterDatabase.Execute(Acore::StringFormat(
            "DELETE FROM item_instance WHERE guid = {}", auction->item_guid.GetCounter()));
    }
}

void AuctionHouseBotWorldScript::OnUpdate(uint32 diff)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    static uint32 updateTimer = 0;
    updateTimer += diff;

    if (updateTimer >= 60000) // Every minute
    {
        updateTimer = 0;
        MarketAnalyzer::Instance().UpdatePrices();
        sAuctionHouseBotMgr.Update(diff);
    }

    static uint32 dailySnapshotTimer = 0;
    dailySnapshotTimer += diff;

    if (dailySnapshotTimer >= 3600000) // Every hour, check for midnight
    {
        dailySnapshotTimer = 0;
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
        if (localTime->tm_hour == 0 && localTime->tm_min < 5)
        {
            MarketAnalyzer::Instance().DailySnapshot();
        }
    }
}

