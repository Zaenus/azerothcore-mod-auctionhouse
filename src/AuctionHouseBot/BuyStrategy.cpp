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

#include "BuyStrategy.h"
#include "AuctionHouseBot.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "Player.h"

BuyStrategy::BuyStrategy(AuctionHouseBot* bot) : _bot(bot)
{
}

void BuyStrategy::Execute()
{
    if (_bot->GetActiveAuctionCount() >= sAuctionHouseConfig.GetMaxActiveAuctionsPerBot())
        return;

    if (_bot->GetGold() < 10000) // Min 1g
        return;

    uint32 maxItems = sAuctionHouseConfig.GetMaxItemsPerCycle();
    uint32 itemsProcessed = 0;

    std::vector<BuyCandidate> candidates;
    ScanAuctions(candidates);

    // Sort by discount (highest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const BuyCandidate& a, const BuyCandidate& b) {
            return a.discountPercent > b.discountPercent;
        });

    for (const auto& candidate : candidates)
    {
        if (itemsProcessed >= maxItems)
            break;

        if (PurchaseAuction(candidate))
        {
            ++itemsProcessed;
            _bot->SetActiveAuctionCount(_bot->GetActiveAuctionCount() + 1);
        }
    }

    if (itemsProcessed > 0)
    {
        LOG_INFO("modules.auctionhouse", "AH Bot (Faction={}) purchased {} items",
            static_cast<uint8>(_bot->GetFaction()), itemsProcessed);
    }
}

void BuyStrategy::ScanAuctions(std::vector<BuyCandidate>& candidates)
{
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

    if (!ah)
        return;

    const auto& blacklisted = sAuctionHouseConfig.GetBlacklistedItems();
    const auto& allowedClasses = sAuctionHouseConfig.GetAllowedItemClasses();

    for (auto const& [id, auction] : ah->GetAuctions())
    {
        // Skip own auctions
        if (auction->owner == _bot->GetBotGuid())
            continue;

        // Skip if already has bidder
        if (auction->bidder)
            continue;

        // Check blacklist
        if (blacklisted.count(auction->item_template))
            continue;

        // Check item class
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(auction->item_template);
        if (!proto)
            continue;

        if (!allowedClasses.empty() && !allowedClasses.count(proto->Class))
            continue;

        // Check item level
        if (proto->ItemLevel < sAuctionHouseConfig.GetMinItemLevel() ||
            proto->ItemLevel > sAuctionHouseConfig.GetMaxItemLevel())
            continue;

        // Skip if no buyout
        if (auction->buyout == 0)
            continue;

        BuyCandidate candidate;
        if (EvaluateAuction(auction, candidate))
        {
            candidates.push_back(candidate);
        }
    }
}

bool BuyStrategy::EvaluateAuction(AuctionEntry* auction, BuyCandidate& candidate)
{
    AuctionHouseFaction faction = static_cast<AuctionHouseFaction>(
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? 0 :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? 1 : 2);

    uint64 marketValue = sMarketAnalyzer.GetMarketValue(auction->item_template, faction);
    if (marketValue == 0)
        return false;

    float maxBuyPercent = sAuctionHouseConfig.GetMaxBuyPricePercent();
    uint64 maxBuyPrice = static_cast<uint64>(marketValue * maxBuyPercent);

    if (auction->buyout > maxBuyPrice)
        return false;

    if (auction->buyout > _bot->GetGold())
        return false;

    float discount = 1.0f - (static_cast<float>(auction->buyout) / static_cast<float>(marketValue));
    if (discount < 0.05f) // At least 5% discount
        return false;

    candidate.auction = auction;
    candidate.marketValue = marketValue;
    candidate.discountPercent = discount;
    candidate.maxBuyPrice = maxBuyPrice;

    return true;
}

bool BuyStrategy::PurchaseAuction(const BuyCandidate& candidate)
{
    AuctionEntry* auction = candidate.auction;
    if (!auction)
        return false;

    // Place bid at buyout price
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

    if (!ah)
        return false;

    // Spend gold
    if (!_bot->SpendGold(auction->buyout))
        return false;

    // Create a fake player for the bid (using bot's character)
    // This is simplified - actual implementation would use proper packet handling
    LOG_INFO("modules.auctionhouse", "AH Bot buying item {} (auction #{}) for {} copper, market value: {}",
        auction->item_template, auction->Id, auction->buyout, candidate.marketValue);

    // TODO: Actually place the bid using auction house system
    // This requires simulating the client packet or calling internal methods

    return true;
}