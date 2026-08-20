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

#include "SellStrategy.h"
#include "AuctionHouseBot.h"
#include "MarketAnalyzer.h"
#include "Config/AuctionHouseConfig.h"
#include "CharacterDatabase.h"
#include "GameTime.h"
#include "Item.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"
#include "Utilities/StringFormat.h"

SellStrategy::SellStrategy(AuctionHouseBot* bot) : _bot(bot)
{
}

void SellStrategy::Execute()
{
    if (_bot->GetActiveAuctionCount() >= sAuctionHouseConfig.GetMaxActiveAuctionsPerBot())
        return;

    std::vector<SellCandidate> candidates;
    ScanInventory(candidates);

    // Sort by market value (highest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const SellCandidate& a, const SellCandidate& b) {
            return a.marketValue > b.marketValue;
        });

    uint32 maxItems = sAuctionHouseConfig.GetMaxItemsPerCycle();
    uint32 itemsProcessed = 0;

    for (const auto& candidate : candidates)
    {
        if (itemsProcessed >= maxItems)
            break;

        if (_bot->GetActiveAuctionCount() >= sAuctionHouseConfig.GetMaxActiveAuctionsPerBot())
            break;

        if (ListItem(candidate))
        {
            ++itemsProcessed;
            _bot->SetActiveAuctionCount(_bot->GetActiveAuctionCount() + 1);
        }
    }

    if (itemsProcessed > 0)
    {
        LOG_INFO("modules.auctionhouse", "AH Bot (Faction={}) listed {} items for sale",
            static_cast<uint8>(_bot->GetFaction()), itemsProcessed);
    }
}

void SellStrategy::ScanInventory(std::vector<SellCandidate>& candidates)
{
    // In a real implementation, this would scan the bot's actual inventory
    // For now, we'll use the bot inventory database table
    std::string query = Acore::StringFormat(
        "SELECT item_entry, count, listed FROM auctionhouse_bot_inventory WHERE bot_guid = {} AND listed = 0",
        _bot->GetBotGuid().GetCounter());

    QueryResult result = CharacterDatabase.Query(query);

    if (!result)
        return;

    const auto& blacklisted = sAuctionHouseConfig.GetBlacklistedItems();
    const auto& allowedClasses = sAuctionHouseConfig.GetAllowedItemClasses();

    do
    {
        Field* fields = result->Fetch();

        uint32 itemEntry = fields[0].Get<uint32>(); // item_entry
        uint32 count = fields[1].Get<uint32>();     // count
        uint8 listed = fields[2].Get<uint8>();      // listed

        // Skip already listed items
        if (listed)
            continue;

        // Check blacklist
        if (blacklisted.count(itemEntry))
            continue;

        // Check item class
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
        if (!proto)
            continue;

        if (!allowedClasses.empty() && !allowedClasses.count(proto->Class))
            continue;

        // Check item level
        if (proto->ItemLevel < sAuctionHouseConfig.GetMinItemLevel() ||
            proto->ItemLevel > sAuctionHouseConfig.GetMaxItemLevel())
            continue;

        SellCandidate candidate;
        if (EvaluateItem(itemEntry, count, candidate))
        {
            candidates.push_back(candidate);
        }
    } while (result->NextRow());
}

bool SellStrategy::EvaluateItem(uint32 itemEntry, uint32 count, SellCandidate& candidate)
{
    AuctionHouseFaction faction = static_cast<AuctionHouseFaction>(
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? 0 :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? 1 : 2);

    uint64 marketValue = MarketAnalyzer::Instance().GetMarketValue(itemEntry, faction);
    if (marketValue == 0)
        return false;

    float minSellPercent = sAuctionHouseConfig.GetMinSellPricePercent();
    uint64 minSellPrice = static_cast<uint64>(marketValue * minSellPercent);

    // Calculate deposit
    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromHouse(
        faction == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        faction == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

    if (!ahEntry)
        return false;

    // Estimate deposit (12h, 24h, 48h). GetAuctionDeposit requires a valid
    // item pointer, so create a temporary one for the calculation.
    uint32 duration = 48 * 60; // 48 hours in minutes

    Item* tempItem = Item::CreateItem(itemEntry, count, nullptr, false, 0, true);
    if (!tempItem)
        return false;

    uint64 deposit = AuctionHouseMgr::GetAuctionDeposit(ahEntry, duration * 60, tempItem, count);
    delete tempItem;

    if (deposit > _bot->GetGold())
        return false;

    candidate.itemEntry = itemEntry;
    candidate.itemCount = count;
    candidate.marketValue = marketValue;
    candidate.minSellPrice = minSellPrice;
    candidate.deposit = deposit;
    candidate.duration = duration;

    return true;
}

bool SellStrategy::ListItem(const SellCandidate& candidate)
{
    AuctionHouseId houseId =
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral;

    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(houseId);
    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromHouse(houseId);

    if (!ah || !ahEntry)
        return false;

    // Spend deposit
    if (!_bot->SpendGold(candidate.deposit))
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(candidate.itemEntry);
    if (!proto)
        return false;

    // Create a virtual item owned by the bot
    Item* virtualItem = Item::CreateItem(candidate.itemEntry, candidate.itemCount, nullptr, false, 0, false);
    if (!virtualItem)
        return false;

    virtualItem->SetOwnerGUID(_bot->GetBotGuid());

    AuctionEntry* auction = new AuctionEntry();
    auction->Id = sObjectMgr->GenerateAuctionID();
    auction->houseId = houseId;
    auction->item_guid = virtualItem->GetGUID();
    auction->item_template = candidate.itemEntry;
    auction->itemCount = candidate.itemCount;
    auction->owner = _bot->GetBotGuid();
    auction->startbid = static_cast<uint32>(candidate.marketValue);
    auction->bidder = ObjectGuid::Empty;
    auction->bid = 0;
    auction->buyout = static_cast<uint32>(candidate.minSellPrice);
    auction->expire_time = GameTime::GetGameTime().count() + static_cast<time_t>(candidate.duration) * 60;
    auction->deposit = static_cast<uint32>(candidate.deposit);
    auction->auctionHouseEntry = ahEntry;

    // Add virtual item to auction house manager
    sAuctionMgr->AddAItem(virtualItem);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    virtualItem->SaveToDB(trans);
    ah->AddAuction(auction);
    auction->SaveToDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    // Item is now listed on the AH, remove it from the bot's virtual inventory
    CharacterDatabase.Execute(Acore::StringFormat(
        "DELETE FROM auctionhouse_bot_inventory WHERE bot_guid = {} AND item_entry = {} AND listed = 0",
        _bot->GetBotGuid().GetCounter(), candidate.itemEntry));

    LOG_INFO("modules.auctionhouse",
        "AH Bot listing item {} x{} for {} copper (market: {}, deposit: {})",
        candidate.itemEntry, candidate.itemCount, candidate.minSellPrice,
        candidate.marketValue, candidate.deposit);

    return true;
}