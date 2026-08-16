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

#ifndef MOD_AUCTIONHOUSE_BLACKMARKET_AH_SCRIPT_H_
#define MOD_AUCTIONHOUSE_BLACKMARKET_AH_SCRIPT_H_

#include "ScriptDefines/AuctionHouseScript.h"
#include "CreatureScript.h"
#include "GossipDef.h"

class BlackMarketAHScript : public CreatureScript, public AuctionHouseScript
{
public:
    BlackMarketAHScript();

    // CreatureScript hooks
    bool OnGossipHello(Player* player, Creature* creature) override;
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override;
    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override;

    // AuctionHouseScript hooks
    void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry) override;
    void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry) override;

private:
    void SendBMAHList(Player* player, Creature* creature);
    void SendBMAHItemDetails(Player* player, Creature* creature, uint32 auctionId);
    bool HandlePlaceBid(Player* player, Creature* creature, const char* code);
};

#endif