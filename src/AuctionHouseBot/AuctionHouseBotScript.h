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

#ifndef MOD_AUCTIONHOUSE_BOT_SCRIPT_H_
#define MOD_AUCTIONHOUSE_BOT_SCRIPT_H_

#include "ScriptDefines/AuctionHouseScript.h"
#include "WorldScript.h"
#include "AuctionHouseBotMgr.h"
#include "MarketAnalyzer.h"

class AuctionHouseBotScript : public AuctionHouseScript
{
public:
    AuctionHouseBotScript() : AuctionHouseScript("AuctionHouseBotScript",
        {
            AUCTIONHOUSEHOOK_ON_AUCTION_ADD,
            AUCTIONHOUSEHOOK_ON_AUCTION_REMOVE,
            AUCTIONHOUSEHOOK_ON_AUCTION_SUCCESSFUL,
            AUCTIONHOUSEHOOK_ON_AUCTION_EXPIRE,
            AUCTIONHOUSEHOOK_ON_BEFORE_AUCTIONHOUSEMGR_UPDATE
        }) {}

    void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnBeforeAuctionHouseMgrUpdate() override;
};

class AuctionHouseBotWorldScript : public WorldScript
{
public:
    AuctionHouseBotWorldScript() : WorldScript("AuctionHouseBotWorldScript",
        std::vector<uint16>{WORLDHOOK_ON_UPDATE, WORLDHOOK_ON_BEFORE_CONFIG_LOAD}) {}

    void OnUpdate(uint32 diff) override;
    void OnBeforeConfigLoad(bool reload) override;
};

#endif