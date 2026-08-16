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

#ifndef MOD_AUCTIONHOUSE_BLACKMARKET_ROTATION_H_
#define MOD_AUCTIONHOUSE_BLACKMARKET_ROTATION_H_

#include "Common.h"
#include "Config/AuctionHouseConfig.h"
#include "BlackMarketAuctionHouse.h"
#include <chrono>
#include <string>

class BlackMarketRotation
{
public:
    BlackMarketRotation();
    ~BlackMarketRotation() = default;

    void Initialize();
    bool Update(uint32 diff);
    bool IsRotationDue() const;
    uint32 StartNewRotation();
    void SaveRotationHistory(const std::vector<BMAHAuctionEntry>& items);

    uint32 GetCurrentRotationId() const { return _currentRotationId; }
    time_t GetNextRotationTime() const { return _nextRotationTime; }

private:
    void CalculateNextRotationTime();
    std::string BuildItemsJson(const std::vector<BMAHAuctionEntry>& items) const;

    uint8 _refreshDay = 3;      // Wednesday
    uint32 _refreshHour = 3;    // 3 AM
    uint32 _currentRotationId = 0;
    time_t _nextRotationTime = 0;
    uint32 _updateTimer = 0;
};

#endif