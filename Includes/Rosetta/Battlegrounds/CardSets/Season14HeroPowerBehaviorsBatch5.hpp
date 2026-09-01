// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

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
    TARGET_TIER_ATTACK,
};

struct Season14HeroPowerBatch5Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch5Kind kind;
    std::int32_t cost;
    bool passive;
};

// These rows are exposed only after their lifecycle and bridge applications
// are implemented against pinned card data.  Unlisted powers remain
// fail-closed and are not credited by coverage tooling.
inline constexpr std::array<Season14HeroPowerBatch5Definition, 3>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH5 = {{
        {"BG28_HERO_400p2", 105395, Season14HeroPowerBatch5Kind::REFRESH_THEN_SEVEN, 0, false},
        {"BG26_HERO_102p", 103501, Season14HeroPowerBatch5Kind::TARGET_TIER_ATTACK, 0, false},
        {"BG26_HERO_102p2", 103503, Season14HeroPowerBatch5Kind::TARGET_TIER_ATTACK, 0, false},
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
    std::int32_t rollCooldown = 0;
    std::int32_t usesThisTurn = 0;
    std::int32_t refreshesThisTurn = 0;
    std::int32_t sellsThisTurn = 0;
    std::int32_t battlecryPurchases = 0;
    std::int32_t combatDeaths = 0;
    std::int32_t enemyKills = 0;
    std::int32_t endTurnBuffLevel = 0;
    std::int32_t friendlyAttacks = 0;
    bool demonHunterTrainingUnlocked = false;
    bool demonHunterTrainingUsedThisTurn = false;
    std::vector<std::int32_t> glaivePurchaseDbfIDs;
    std::int32_t glaiveUsesRemaining = 3;
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
    FRIENDLY_MINION_ATTACKED,
};

struct Season14HeroPowerBatch5Result
{
    std::int32_t amount = 0;
    std::int32_t goldDelta = 0;
    std::int32_t extraDragonOffers = 0;
    std::int32_t tavernSlotsDelta = 0;
    std::int32_t attack = 0;
    std::int32_t health = 0;
    std::int32_t summonAttack = 0;
    std::int32_t summonHealth = 0;
    bool trigger = false;
};

constexpr bool Season14HeroPowerBatch5FirstBuyFree(
    const Season14HeroPowerBatch5State& state) noexcept
{
    return state.demonHunterTrainingUnlocked &&
           !state.demonHunterTrainingUsedThisTurn;
}

constexpr bool ConsumeSeason14HeroPowerBatch5FirstBuyFree(
    std::int32_t dbfID, Season14HeroPowerBatch5State& state) noexcept
{
    if (dbfID != 61915 || !Season14HeroPowerBatch5FirstBuyFree(state))
        return false;
    state.demonHunterTrainingUsedThisTurn = true;
    return true;
}

constexpr bool Season14HeroPowerBatch5GlaiveReady(
    const Season14HeroPowerBatch5State& state) noexcept
{
    return state.glaiveUsesRemaining > 0 &&
           state.glaivePurchaseDbfIDs.size() >= 3;
}

inline void RecordSeason14HeroPowerBatch5GlaivePurchase(
    std::int32_t dbfID, Season14HeroPowerBatch5State& state)
{
    if (dbfID <= 0) return;
    state.glaivePurchaseDbfIDs.push_back(dbfID);
    if (state.glaivePurchaseDbfIDs.size() > 3)
        state.glaivePurchaseDbfIDs.erase(state.glaivePurchaseDbfIDs.begin());
}

constexpr bool ConsumeSeason14HeroPowerBatch5Glaive(
    Season14HeroPowerBatch5State& state) noexcept
{
    if (!Season14HeroPowerBatch5GlaiveReady(state)) return false;
    --state.glaiveUsesRemaining;
    state.glaivePurchaseDbfIDs.clear();
    return true;
}

constexpr bool ResolveSeason14HeroPowerBatch5Activation(
    std::int32_t dbfID, Season14HeroPowerBatch5State& state,
    Season14HeroPowerBatch5Result& result, std::int32_t roll = 1,
    std::int32_t tier = 1) noexcept
{
    result = {};
    const auto* definition = FindSeason14HeroPowerBehaviorBatch5(dbfID);
    if (definition == nullptr || definition->passive) return false;
    if (definition->kind == Season14HeroPowerBatch5Kind::REFRESH_THEN_SEVEN)
    {
        if (state.rollCooldown != 0 || roll < 1 || roll > 6) return false;
        result.goldDelta = roll;
        result.trigger = true;
        state.rollCooldown = roll;
        return true;
    }
    if (definition->kind == Season14HeroPowerBatch5Kind::BATTLECRY_PURCHASE_COUNTER)
    {
        result.trigger = true;
        result.amount = 1;
        return true;
    }
    if (definition->kind == Season14HeroPowerBatch5Kind::TARGET_TIER_ATTACK)
    {
        if (state.usesThisTurn >= 2 || tier < 1) return false;
        ++state.usesThisTurn;
        if (dbfID == 103503)
            result.health = tier;
        else
            result.attack = tier;
        result.trigger = true;
        return true;
    }
    return false;
}

//! Resolve deterministic lifecycle bookkeeping.  Random recipients and
//! generated card creation remain the responsibility of the caller.
constexpr void ResolveSeason14HeroPowerBatch5Event(
    std::int32_t dbfID, Season14HeroPowerBatch5Event event,
    Season14HeroPowerBatch5State& state,
    Season14HeroPowerBatch5Result& result) noexcept
{
    result = {};
    if (FindSeason14HeroPowerBehaviorBatch5(dbfID) == nullptr)
    {
        return;
    }
    if (event == Season14HeroPowerBatch5Event::BEGIN_TURN)
    {
        state.refreshesThisTurn = 0;
        state.sellsThisTurn = 0;
        if (state.rollCooldown > 0) --state.rollCooldown;
        state.usesThisTurn = 0;
        state.demonHunterTrainingUsedThisTurn = false;
        state.glaivePurchaseDbfIDs.clear();
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
    if (event == Season14HeroPowerBatch5Event::FRIENDLY_MINION_ATTACKED &&
        dbfID == 61915 && !state.demonHunterTrainingUnlocked)
    {
        if (++state.friendlyAttacks >= 14)
            state.demonHunterTrainingUnlocked = true;
    }
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH5_HPP
