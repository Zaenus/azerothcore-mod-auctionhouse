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

#include "BlackMarketMgr.h"
#include "BlackMarketAuctionHouse.h"
#include "BlackMarketItemPool.h"
#include "BlackMarketRotation.h"
#include "BlackMarketBidManager.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "ObjectMgr.h"
#include "World.h"

void BlackMarketMgr::Initialize()
{
    if (!sAuctionHouseConfig.IsBMAHEnabled())
    {
        LOG_INFO("modules.auctionhouse", "Black Market AH is disabled");
        return;
    }

    _enabled = true;
    _npcEntry = sAuctionHouseConfig.GetBMAHNPCEntry();

    _itemPool = std::make_unique<BlackMarketItemPool>();
    _itemPool->LoadFromDB();

    _rotation = std::make_unique<BlackMarketRotation>();
    _rotation->Initialize();

    _bidManager = std::make_unique<BlackMarketBidManager>();

    CreateAuctionHouse();
    LoadFromDB();

    // Check if rotation is needed
    if (_rotation->IsRotationDue())
    {
        SpawnRotationItems();
    }

    LOG_INFO("modules.auctionhouse", "Black Market AH initialized with {} active auctions",
        _auctionHouse ? _auctionHouse->Getcount() : 0);
}

void BlackMarketMgr::CreateAuctionHouse()
{
    _auctionHouse = std::make_unique<BlackMarketAuctionHouse>();
}

void BlackMarketMgr::Update(uint32 diff)
{
    if (!_enabled)
        return;

    std::lock_guard lock(_mutex);

    // Update rotation timer
    if (_rotation->Update(diff))
    {
        SpawnRotationItems();
    }

    // Update bid manager
    _bidManager->Update(diff);

    // Update auction house (expire old auctions)
    if (_auctionHouse)
    {
        _auctionHouse->Update();
    }
}

void BlackMarketMgr::LoadFromDB()
{
    if (!_auctionHouse)
        return;

    // Load active auctions from database
    // This would query blackmarket_auctions table
}

void BlackMarketMgr::SaveToDB()
{
    // Save active auctions to database
}

void BlackMarketMgr::SpawnRotationItems()
{
    if (!_auctionHouse || !_itemPool || !_rotation)
        return;

    // Clear expired auctions first
    _auctionHouse->ClearExpiredAuctions();

    // Start new rotation
    uint32 rotationId = _rotation->StartNewRotation();

    // Select items for this rotation
    std::vector<BMAHAuctionEntry> newAuctions = _itemPool->SelectItemsForRotation(
        sAuctionHouseConfig.GetBMAHMaxActiveAuctions(),
        rotationId);

    // Create auction entries
    for (auto& auctionData : newAuctions)
    {
        // Create item
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(auctionData.itemEntry);
        if (!proto)
            continue;

        Item* item = Item::CreateItem(auctionData.itemEntry, auctionData.itemCount);
        if (!item)
            continue;

        // Set auction data
        auctionData.itemGuid = item->GetGUID();

        // Add to auction house
        _auctionHouse->AddAuction(auctionData);

        // Save to DB
        // TODO: Save to blackmarket_auctions table
    }

    _rotation->SaveRotationHistory(newAuctions);

    LOG_INFO("modules.auctionhouse", "Black Market AH rotation {} spawned {} new auctions",
        rotationId, newAuctions.size());
}

void BlackMarketMgr::OnAuctionExpired(uint32 auctionId)
{
    // Handle expired auction (return item to "system")
    LOG_INFO("modules.auctionhouse", "BMAH auction {} expired", auctionId);
}

void BlackMarketMgr::OnAuctionWon(uint32 auctionId, ObjectGuid winnerGuid, uint64 bidAmount)
{
    // Send item to winner via mail
    LOG_INFO("modules.auctionhouse", "BMAH auction {} won by {} for {} copper",
        auctionId, winnerGuid.GetCounter(), bidAmount);
}

void BlackMarketMgr::OnBidPlaced(uint32 auctionId, ObjectGuid bidderGuid, uint64 bidAmount)
{
    LOG_INFO("modules.auctionhouse", "BMAH auction {} bid by {} for {} copper",
        auctionId, bidderGuid.GetCounter(), bidAmount);
}