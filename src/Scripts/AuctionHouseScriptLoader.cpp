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

#include "AuctionHouseBot/AuctionHouseBotScript.h"
#include "Config/AuctionHouseConfig.h"
#include "ScriptMgr.h"

// AH Bot scripts
void AddSC_AuctionHouseBotScript()
{
    new AuctionHouseBotScript();
    new AuctionHouseBotWorldScript();
}

// Module entry point - called by script loader (matches mod-auctionhouse -> mod_auctionhouse)
void Addmod_auctionhouseScripts()
{
    sAuctionHouseConfig.Initialize(false);
    MarketAnalyzer::Instance().Initialize();
    sAuctionHouseBotMgr.Initialize();

    AddSC_AuctionHouseBotScript();
}