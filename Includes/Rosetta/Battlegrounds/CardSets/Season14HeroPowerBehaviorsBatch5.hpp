// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Target-free recruit/combat lifecycle powers for the accelerated
//! experimental pool.  The resolver intentionally contains only deterministic
//! bookkeeping; callers must apply the returned deltas at the corresponding
//! game boundary.
enum class Season14HeroPowerBatch5Kind : std::uint8_t
{
    EXTRA_DRAGON_REFRESH,
    REFRESH_THEN_SEVEN,
    FROZEN_MINION_BUFF,
    SELL_TAVERN_BUFF,
    BATTLECRY_PURCHASE_COUNTER,
    END_TURN_SCALING_BUFF,
    COMBAT_SUMMON_AURA,
    AVENGE_WHELP,
    ENEMY_KILL_COUNTER,
};

struct Season14HeroPowerBatch5Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch5Kind kind;
    std::int32_t cost;
    bool passive;
};

//! Exact Patch 36.4 IDs.  These powers are all fixed-lifecycle families;
//! choice/discover and random-selection powers remain outside this batch.
inline constexpr std::array<Season14HeroPowerBatch5Definition, 8>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH5 = {{
        {"TB_BaconShop_HP_062", 61408,
         Season14HeroPowerBatch5Kind::EXTRA_DRAGON_REFRESH, 0, true},
        {"TB_BaconShop_HP_065", 61915,
         Season14HeroPowerBatch5Kind::REFRESH_THEN_SEVEN, 0, true},
        {"TB_BaconShop_HP_042", 59860,
         Season14HeroPowerBatch5Kind::SELL_TAVERN_BUFF, 0, true},
        {"TB_BaconShop_HP_048", 60218,
         Season14HeroPowerBatch5Kind::BATTLECRY_PURCHASE_COUNTER, 0, true},
        {"TB_BaconShop_HP_087t", 64426,
         Season14HeroPowerBatch5Kind::END_TURN_SCALING_BUFF, 0, true},
        {"TB_BaconShop_HP_107", 67554,
         Season14HeroPowerBatch5Kind::COMBAT_SUMMON_AURA, 0, true},
        {"BG22_HERO_305p", 82114,
         Season14HeroPowerBatch5Kind::AVENGE_WHELP, 0, true},
        {"TB_BaconShop_HP_087", 64424,
         Season14HeroPowerBatch5Kind::ENEMY_KILL_COUNTER, 0, true},
    }};

constexpr const Season14HeroPowerBatch5Definition*
FindSeason14HeroPowerBehaviorBatch5(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH5)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerBatch5Definition*
FindSeason14HeroPowerBehaviorBatch5(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH5)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

struct Season14HeroPowerBatch5State
{
    std::int32_t refreshesThisTurn = 0;
    std::int32_t sellsThisTurn = 0;
    std::int32_t battlecryPurchases = 0;
    std::int32_t combatDeaths = 0;
    std::int32_t enemyKills = 0;
    std::int32_t endTurnBuffLevel = 0;
};

enum class Season14HeroPowerBatch5Event : std::uint8_t
{
    BEGIN_TURN,
    REFRESH_TAVERN,
    SELL_MINION,
    BUY_BATTLECRY_MINION,
    END_TURN,
    COMBAT_START,
    FRIENDLY_MINION_DIED,
    ENEMY_MINION_KILLED,
};

struct Season14HeroPowerBatch5Result
{
    std::int32_t extraDragonOffers = 0;
    std::int32_t tavernSlotsDelta = 0;
    std::int32_t attack = 0;
    std::int32_t health = 0;
    std::int32_t summonAttack = 0;
    std::int32_t summonHealth = 0;
    bool trigger = false;
};

//! Resolve deterministic lifecycle bookkeeping.  Random recipients and
//! generated card creation remain the responsibility of the caller.
constexpr void ResolveSeason14HeroPowerBatch5Event(
    std::int32_t dbfID, Season14HeroPowerBatch5Event event,
    Season14HeroPowerBatch5State& state,
    Season14HeroPowerBatch5Result& result) noexcept
{
    result = {};
    if (event == Season14HeroPowerBatch5Event::BEGIN_TURN)
    {
        state.refreshesThisTurn = 0;
        state.sellsThisTurn = 0;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::REFRESH_TAVERN)
    {
        ++state.refreshesThisTurn;
        if (dbfID == 61408)
        {
            result.extraDragonOffers = 1;
            result.trigger = true;
        }
        if (dbfID == 61915 && state.refreshesThisTurn >= 5)
        {
            result.tavernSlotsDelta = 1;
            result.trigger = true;
        }
        return;
    }
    if (event == Season14HeroPowerBatch5Event::SELL_MINION)
    {
        ++state.sellsThisTurn;
        if (dbfID == 59860)
        {
            result.attack = 1;
            result.health = 1;
            result.trigger = true;
        }
        return;
    }
    if (event == Season14HeroPowerBatch5Event::BUY_BATTLECRY_MINION &&
        dbfID == 60218)
    {
        ++state.battlecryPurchases;
        result.trigger = state.battlecryPurchases >= 5;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::END_TURN)
    {
        if (dbfID == 64426)
        {
            ++state.endTurnBuffLevel;
            result.attack = 3;
            result.health = 3;
            result.trigger = true;
        }
        return;
    }
    if (event == Season14HeroPowerBatch5Event::COMBAT_START)
    {
        state.combatDeaths = 0;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::ENEMY_MINION_KILLED &&
        dbfID == 64424)
    {
        ++state.enemyKills;
        result.trigger = state.enemyKills >= 25;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::FRIENDLY_MINION_DIED)
    {
        ++state.combatDeaths;
        if (dbfID == 82114 && state.combatDeaths >= 4)
        {
            state.combatDeaths = 0;
            result.summonAttack = 3;
            result.summonHealth = 1;
            result.trigger = true;
        }
    }
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP
