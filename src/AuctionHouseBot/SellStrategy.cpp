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
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "Player.h"

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
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AH_BOT_INVENTORY);
    stmt->SetData(0, _bot->GetBotGuid().GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    const auto& blacklisted = sAuctionHouseConfig.GetBlacklistedItems();
    const auto& allowedClasses = sAuctionHouseConfig.GetAllowedItemClasses();

    do
    {
        Field* fields = result->Fetch();

        uint32 itemEntry = fields[1].Get<uint32>(); // item_entry
        uint32 count = fields[2].Get<uint32>();     // count
        uint8 listed = fields[5].Get<uint8>();      // listed

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

    uint64 marketValue = sMarketAnalyzer.GetMarketValue(itemEntry, faction);
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

    // Estimate deposit (12h, 24h, 48h)
    uint32 duration = 48 * 60; // 48 hours in minutes
    uint64 deposit = AuctionHouseMgr::GetAuctionDeposit(ahEntry, duration * 60, nullptr, count);

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
    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(
        _bot->GetFaction() == AuctionHouseFaction::Alliance ? AuctionHouseId::Alliance :
        _bot->GetFaction() == AuctionHouseFaction::Horde ? AuctionHouseId::Horde : AuctionHouseId::Neutral);

    if (!ah)
        return false;

    // Spend deposit
    if (!_bot->SpendGold(candidate.deposit))
        return false;

    LOG_INFO("modules.auctionhouse", "AH Bot listing item {} x{} for {} copper (market: {}, deposit: {})",
        candidate.itemEntry, candidate.itemCount, candidate.minSellPrice,
        candidate.marketValue, candidate.deposit);

    // TODO: Actually create the auction using auction house system
    // This requires creating an AuctionEntry and calling AddAuction

    return true;
}