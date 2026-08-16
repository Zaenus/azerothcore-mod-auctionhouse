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
#include "Logging/Log.h"

void AuctionHouseBotScript::OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    sMarketAnalyzer.RecordListing(ah, entry);
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

    sMarketAnalyzer.RecordSale(ah, entry);
}

void AuctionHouseBotScript::OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    sMarketAnalyzer.RecordExpiry(ah, entry);
}

void AuctionHouseBotScript::OnBeforeAuctionHouseMgrUpdate()
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    sAuctionHouseBotMgr.Update(60000); // Called every minute by AH mgr
}

void AuctionHouseBotWorldScript::OnWorldUpdate(uint32 diff)
{
    if (!sAuctionHouseConfig.IsAHBotEnabled())
        return;

    static uint32 updateTimer = 0;
    updateTimer += diff;

    if (updateTimer >= 60000) // Every minute
    {
        updateTimer = 0;
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
            sMarketAnalyzer.DailySnapshot();
        }
    }
}

void AuctionHouseBotWorldScript::OnBeforeConfigLoad(bool reload)
{
    sAuctionHouseConfig.Initialize(reload);

    if (reload)
    {
        sAuctionHouseBotMgr.Update(0); // Trigger re-initialization
    }
}