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

#include "BlackMarketAHScript.h"
#include "BlackMarketMgr.h"
#include "BlackMarketAuctionHouse.h"
#include "BlackMarketBidManager.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "Player.h"
#include "Creature.h"
#include "GossipDef.h"
#include "ObjectAccessor.h"
#include "Chat.h"
#include "ScriptedGossip.h"

BlackMarketAHScript::BlackMarketAHScript()
    : CreatureScript("npc_blackmarket_auctioneer"),
      AuctionHouseScript("BlackMarketAHScript",
          {
              AUCTIONHOUSEHOOK_ON_AUCTION_ADD,
              AUCTIONHOUSEHOOK_ON_AUCTION_REMOVE,
              AUCTIONHOUSEHOOK_ON_AUCTION_SUCCESSFUL,
              AUCTIONHOUSEHOOK_ON_AUCTION_EXPIRE
          })
{
}

bool BlackMarketAHScript::OnGossipHello(Player* player, Creature* creature)
{
    if (!sAuctionHouseConfig.IsBMAHEnabled())
        return false;

    if (creature->GetEntry() != sAuctionHouseConfig.GetBMAHNPCEntry())
        return false;

    SendBMAHList(player, creature);
    return true;
}

bool BlackMarketAHScript::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (!sAuctionHouseConfig.IsBMAHEnabled())
        return false;

    if (sender == GOSSIP_SENDER_MAIN)
    {
        switch (action)
        {
            case 1: // View item details
                // Action stores auction ID in high bits
                SendBMAHItemDetails(player, creature, action >> 16);
                break;
            case 2: // Place bid
                player->PrepareGossipMenu(creature, 1000, true);
                player->SendPreparedGossip(creature);
                break;
        }
    }
    else if (sender == GOSSIP_SENDER_MAIN + 1)
    {
        // Bid confirmation
        HandlePlaceBid(player, creature, std::to_string(action).c_str());
    }

    return true;
}

bool BlackMarketAHScript::OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
{
    if (sender == GOSSIP_SENDER_MAIN + 1)
    {
        HandlePlaceBid(player, creature, code);
    }
    return true;
}

void BlackMarketAHScript::SendBMAHList(Player* player, Creature* creature)
{
    BlackMarketAuctionHouse* bmah = sBlackMarketMgr.GetAuctionHouse();
    if (!bmah)
    {
        ChatHandler(player->GetSession()).SendNotification("Black Market Auction House is currently unavailable.");
        return;
    }

    const auto& auctions = bmah->GetAuctions();
    if (auctions.empty())
    {
        ChatHandler(player->GetSession()).SendNotification("The Black Market is currently empty. Check back next rotation!");
        return;
    }

    std::ostringstream menu;
    menu << "Black Market Auction House\n\n";
    menu << "Current Auctions: " << auctions.size() << "\n\n";

    for (const auto& [id, auction] : auctions)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(auction.itemEntry);
        if (!proto)
            continue;

        time_t timeLeft = auction.expireTime - time(nullptr);
        uint32 hoursLeft = static_cast<uint32>(timeLeft / 3600);
        uint32 minsLeft = static_cast<uint32>((timeLeft % 3600) / 60);

        menu << "|cffffffff" << proto->Name1 << "|r\n";
        menu << "  Current Bid: " << auction.currentBid / 10000 << "g "
             << (auction.currentBid % 10000) / 100 << "s "
             << auction.currentBid % 100 << "c\n";
        menu << "  Time Left: " << hoursLeft << "h " << minsLeft << "m\n";
        menu << "  [View Details] (Action: " << ((id << 16) | 1) << ")\n\n";
    }

    ChatHandler(player->GetSession()).SendNotification(menu.str().c_str());
}

void BlackMarketAHScript::SendBMAHItemDetails(Player* player, Creature* creature, uint32 auctionId)
{
    BlackMarketAuctionHouse* bmah = sBlackMarketMgr.GetAuctionHouse();
    if (!bmah)
        return;

    BMAHAuctionEntry* auction = bmah->GetAuction(auctionId);
    if (!auction)
        return;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(auction->itemEntry);
    if (!proto)
        return;

    time_t timeLeft = auction->expireTime - time(nullptr);
    uint32 hoursLeft = static_cast<uint32>(timeLeft / 3600);
    uint32 minsLeft = static_cast<uint32>((timeLeft % 3600) / 60);

    float minIncrement = sAuctionHouseConfig.GetBMAHBidIncrementPercent();
    uint64 minBid = auction->currentBid + static_cast<uint64>(auction->currentBid * minIncrement);
    if (auction->currentBid == 0)
        minBid = auction->startBid;

    std::ostringstream details;
    details << "|cffffffff" << proto->Name1 << "|r\n\n";
    details << "Item Level: " << proto->ItemLevel << "\n";
    details << "Quality: " << proto->Quality << "\n";
    details << "Current Bid: " << auction->currentBid / 10000 << "g "
         << (auction->currentBid % 10000) / 100 << "s "
         << auction->currentBid % 100 << "c\n";
    details << "Minimum Next Bid: " << minBid / 10000 << "g "
         << (minBid % 10000) / 100 << "s "
         << minBid % 100 << "c\n";
    details << "Time Left: " << hoursLeft << "h " << minsLeft << "m\n\n";
    details << "Enter bid amount (in copper):";

    ChatHandler(player->GetSession()).SendNotification(details.str().c_str());

    // Prompt for bid input
    player->PrepareGossipMenu(creature, 1001, true);
    player->SendPreparedGossip(creature);
}

bool BlackMarketAHScript::HandlePlaceBid(Player* player, Creature* creature, const char* code)
{
    if (!code || !*code)
        return false;

    uint64 bidAmount = 0;
    try
    {
        bidAmount = std::stoull(code);
    }
    catch (...)
    {
        ChatHandler(player->GetSession()).SendNotification("Invalid bid amount.");
        return false;
    }

    if (bidAmount == 0)
    {
        ChatHandler(player->GetSession()).SendNotification("Bid amount must be greater than 0.");
        return false;
    }

    // Get auction ID from context (stored in player session or similar)
    // For simplicity, we'll use a placeholder - in reality you'd track this
    uint32 auctionId = 0; // TODO: Get from player session data

    std::string errorMsg;
    bool success = sBlackMarketMgr.GetBidManager()->PlaceBid(auctionId, player->GetGUID(), bidAmount, errorMsg);

    if (success)
    {
        ChatHandler(player->GetSession()).SendNotification("Your bid of " + std::to_string(bidAmount / 10000) + "g has been placed!");
    }
    else
    {
        ChatHandler(player->GetSession()).SendNotification("Bid failed: " + errorMsg);
    }

    return true;
}

void BlackMarketAHScript::OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry)
{
    // Track BMAH auctions separately
}

void BlackMarketAHScript::OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry)
{
}

void BlackMarketAHScript::OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry)
{
}

void BlackMarketAHScript::OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry)
{
}