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

#include "BlackMarketBidManager.h"
#include "BlackMarketAuctionHouse.h"
#include "BlackMarketMgr.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "Player.h"

void BlackMarketBidManager::Update(uint32 diff)
{
    _updateTimer += diff;

    if (_updateTimer < 5000) // Process every 5 seconds
        return;

    _updateTimer = 0;

    // Process pending bids (if any async processing needed)
    for (auto& bid : _pendingBids)
    {
        // Bid processing logic
    }
    _pendingBids.clear();
}

bool BlackMarketBidManager::PlaceBid(uint32 auctionId, ObjectGuid bidderGuid, uint64 bidAmount, std::string& errorMsg)
{
    BlackMarketAuctionHouse* bmah = sBlackMarketMgr.GetAuctionHouse();
    if (!bmah)
    {
        errorMsg = "Black Market AH not available";
        return false;
    }

    BMAHAuctionEntry* auction = bmah->GetAuction(auctionId);
    if (!auction)
    {
        errorMsg = "Auction not found";
        return false;
    }

    // Check if auction is still active
    time_t now = time(nullptr);
    if (auction->expireTime <= now)
    {
        errorMsg = "Auction has expired";
        return false;
    }

    // Check minimum bid increment
    float minIncrement = sAuctionHouseConfig.GetBMAHBidIncrementPercent();
    uint64 minBid = auction->currentBid + static_cast<uint64>(auction->currentBid * minIncrement);
    if (auction->currentBid == 0)
        minBid = auction->startBid;

    if (bidAmount < minBid)
    {
        errorMsg = "Bid too low. Minimum bid: " + std::to_string(minBid);
        return false;
    }

    // Check if player has enough gold
    Player* player = ObjectAccessor::FindConnectedPlayer(bidderGuid);
    if (!player)
    {
        errorMsg = "Player not found";
        return false;
    }

    if (player->GetMoney() < bidAmount)
    {
        errorMsg = "Not enough gold";
        return false;
    }

    // Refund previous bidder if outbid
    if (auction->bidderGuid && auction->bidderGuid != bidderGuid)
    {
        Player* oldBidder = ObjectAccessor::FindConnectedPlayer(auction->bidderGuid);
        if (oldBidder)
        {
            oldBidder->ModifyMoney(auction->currentBid);
        }
        else
        {
            // Offline player - send mail
            // TODO: Implement mail refund
        }
    }

    // Deduct gold from new bidder
    player->ModifyMoney(-static_cast<int64>(bidAmount));

    // Update auction
    auction->currentBid = bidAmount;
    auction->bidderGuid = bidderGuid;

    LOG_INFO("modules.auctionhouse", "BMAH bid placed: auction #{}, bidder {}, amount {}",
        auctionId, bidderGuid.GetCounter(), bidAmount);

    return true;
}

void BlackMarketBidManager::OnAuctionEnded(uint32 auctionId, bool won, ObjectGuid winnerGuid, uint64 finalBid)
{
    BlackMarketAuctionHouse* bmah = sBlackMarketMgr.GetAuctionHouse();
    if (!bmah)
        return;

    BMAHAuctionEntry* auction = bmah->GetAuction(auctionId);
    if (!auction)
        return;

    if (won && winnerGuid)
    {
        // Send item to winner via mail
        // TODO: Implement mail delivery
        LOG_INFO("modules.auctionhouse", "BMAH auction #{} won by {} for {} copper",
            auctionId, winnerGuid.GetCounter(), finalBid);
    }
    else
    {
        // Auction expired, item returns to "system"
        LOG_INFO("modules.auctionhouse", "BMAH auction #{} expired", auctionId);
    }

    // Remove from active auctions
    bmah->RemoveAuction(auctionId);
}