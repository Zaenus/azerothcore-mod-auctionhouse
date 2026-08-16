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

#include "BlackMarketRotation.h"
#include "BlackMarketAuctionHouse.h"
#include "Config/AuctionHouseConfig.h"
#include "Logging/Log.h"
#include "WorldDatabase.h"
#include <ctime>
#include <sstream>
#include <iomanip>

BlackMarketRotation::BlackMarketRotation()
{
}

void BlackMarketRotation::Initialize()
{
    _refreshDay = sAuctionHouseConfig.GetBMAHRefreshDay();
    _refreshHour = sAuctionHouseConfig.GetBMAHRefreshHour();

    // Load last rotation from DB
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_BMAH_LAST_ROTATION);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (result)
    {
        Field* fields = result->Fetch();
        _currentRotationId = fields[0].Get<uint32>();
    }

    CalculateNextRotationTime();

    LOG_INFO("modules.auctionhouse", "BMAH Rotation initialized: current={}, next={}",
        _currentRotationId, _nextRotationTime);
}

bool BlackMarketRotation::Update(uint32 diff)
{
    _updateTimer += diff;

    if (_updateTimer < 60000) // Check every minute
        return false;

    _updateTimer = 0;

    time_t now = time(nullptr);
    if (now >= _nextRotationTime)
    {
        return true; // Rotation due
    }

    return false;
}

bool BlackMarketRotation::IsRotationDue() const
{
    time_t now = time(nullptr);
    return now >= _nextRotationTime;
}

uint32 BlackMarketRotation::StartNewRotation()
{
    ++_currentRotationId;
    CalculateNextRotationTime();

    LOG_INFO("modules.auctionhouse", "BMAH Rotation {} started, next at {}", _currentRotationId, _nextRotationTime);
    return _currentRotationId;
}

void BlackMarketRotation::SaveRotationHistory(const std::vector<BMAHAuctionEntry>& items)
{
    std::string itemsJson = BuildItemsJson(items);

    WorldDatabaseTransaction trans = WorldDatabase.BeginTransaction();

    // Insert rotation history
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_BMAH_ROTATION_HISTORY);
    stmt->SetData(0, _currentRotationId);
    stmt->SetData(1, itemsJson);
    trans->Append(stmt);

    // Update last rotation id
    stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_BMAH_LAST_ROTATION);
    stmt->SetData(0, _currentRotationId);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

void BlackMarketRotation::CalculateNextRotationTime()
{
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    // Create target time for next refresh day/hour
    tm targetTime = {};
    targetTime.tm_year = localTime->tm_year;
    targetTime.tm_mon = localTime->tm_mon;
    targetTime.tm_mday = localTime->tm_mday;
    targetTime.tm_hour = _refreshHour;
    targetTime.tm_min = 0;
    targetTime.tm_sec = 0;
    targetTime.tm_isdst = localTime->tm_isdst;

    // Find next occurrence of refresh day
    int daysUntilRefresh = (_refreshDay - localTime->tm_wday + 7) % 7;
    if (daysUntilRefresh == 0 && localTime->tm_hour >= _refreshHour)
    {
        daysUntilRefresh = 7; // Next week
    }

    targetTime.tm_mday += daysUntilRefresh;
    _nextRotationTime = mktime(&targetTime);
}

std::string BlackMarketRotation::BuildItemsJson(const std::vector<BMAHAuctionEntry>& items) const
{
    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < items.size(); ++i)
    {
        if (i > 0)
            oss << ",";

        oss << "{"
            << "\"itemEntry\":" << items[i].itemEntry << ","
            << "\"itemCount\":" << items[i].itemCount << ","
            << "\"startBid\":" << items[i].startBid << ","
            << "\"poolCategory\":\"" << items[i].poolCategory << "\""
            << "}";
    }

    oss << "]";
    return oss.str();
}