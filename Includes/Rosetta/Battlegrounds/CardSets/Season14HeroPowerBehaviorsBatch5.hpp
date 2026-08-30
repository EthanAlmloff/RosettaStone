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

// The first draft of this batch registered eight rows with guessed
// thresholds/effects.  The pinned card data disproves those semantics (for
// example, Demon Hunter Training is 14 attacks and a first-buy-free trigger,
// while the old resolver counted refreshes).  Keep the family types and
// state-machine scaffolding available for the next implementation pass, but
// do not expose any row as implemented until its Player lifecycle and bridge
// application are complete.  An empty registry is intentional fail-closed
// behavior and prevents coverage tooling from crediting partial effects.
inline constexpr std::array<Season14HeroPowerBatch5Definition, 0>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH5 = {{}};

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
    // An empty registry is deliberate until a complete Player/bridge
    // applicator is reviewed.  Keep even bookkeeping inert for unregistered
    // IDs so this scaffolding cannot influence a live game accidentally.
    if (FindSeason14HeroPowerBehaviorBatch5(dbfID) == nullptr)
    {
        return;
    }
    if (event == Season14HeroPowerBatch5Event::BEGIN_TURN)
    {
        state.refreshesThisTurn = 0;
        state.sellsThisTurn = 0;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::REFRESH_TAVERN)
    {
        ++state.refreshesThisTurn;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::SELL_MINION)
    {
        ++state.sellsThisTurn;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::BUY_BATTLECRY_MINION)
    {
        ++state.battlecryPurchases;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::END_TURN)
    {
        return;
    }
    if (event == Season14HeroPowerBatch5Event::COMBAT_START)
    {
        state.combatDeaths = 0;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::ENEMY_MINION_KILLED)
    {
        ++state.enemyKills;
        return;
    }
    if (event == Season14HeroPowerBatch5Event::FRIENDLY_MINION_DIED)
    {
        ++state.combatDeaths;
    }
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP
