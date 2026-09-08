// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH8_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH8_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Small, deterministic Patch 36.4 hero-power families.  This registry owns
//! only the exact cost/passive metadata and deterministic lifecycle deltas.
//! Target selection, random recipients, and generated cards remain the
//! caller's responsibility; no unsupported modal is silently synthesized.
enum class Season14HeroPowerBatch8Kind : std::uint8_t
{
    SKILLED_BARTENDER,
    MURLOC_KING,
    RAGE_POTION,
    DIE_INSECTS,
    FIRE_CANNONS,
    BANANARAMA,
    NEFARIOUS_FIRE,
    HONORABLE_WARBAND,
};

struct Season14HeroPowerBatch8Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch8Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch8Definition, 8>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH8 = {{
        {"TB_BaconShop_HP_009", 57561, Season14HeroPowerBatch8Kind::SKILLED_BARTENDER, 0, true},
        {"TB_BaconShop_HP_017", 57957, Season14HeroPowerBatch8Kind::MURLOC_KING, 1, false},
        {"TB_BaconShop_HP_018", 57962, Season14HeroPowerBatch8Kind::RAGE_POTION, 1, false},
        {"TB_BaconShop_HP_019", 58017, Season14HeroPowerBatch8Kind::DIE_INSECTS, 2, true},
        {"TB_BaconShop_HP_027", 58527, Season14HeroPowerBatch8Kind::FIRE_CANNONS, 1, true},
        {"TB_BaconShop_HP_038", 59815, Season14HeroPowerBatch8Kind::BANANARAMA, 1, false},
        {"TB_BaconShop_HP_043", 59862, Season14HeroPowerBatch8Kind::NEFARIOUS_FIRE, 1, true},
        {"TB_BaconShop_HP_051", 60376, Season14HeroPowerBatch8Kind::HONORABLE_WARBAND, 2, false},
    }};

constexpr const Season14HeroPowerBatch8Definition*
FindSeason14HeroPowerBehaviorBatch8(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH8)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

constexpr const Season14HeroPowerBatch8Definition*
FindSeason14HeroPowerBehaviorBatch8(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH8)
        if (definition.id == id) return &definition;
    return nullptr;
}

struct Season14HeroPowerBatch8State
{
    std::int32_t turnNumber = 0;
    std::int32_t sales = 0;
    std::int32_t pendingGold = 0;
    bool bananaramaPending = false;
    bool murlocKingPending = false;
    std::int32_t pendingStartCombatPower = 0;
    // Keep activation multiplicity explicit: Ancient Wishbone duplicates an
    // active power's effect, and a bool would silently collapse the copies.
    std::int32_t bananaramaActivations = 0;
    std::int32_t murlocKingActivations = 0;
    std::int32_t pendingStartCombatPowerRepeats = 0;
};

enum class Season14HeroPowerBatch8Event : std::uint8_t
{
    BEGIN_TURN,
    BUY_CARD,
    SELL_MINION,
    PLAY_ELEMENTAL,
    UPGRADE_TAVERN,
};

struct Season14HeroPowerBatch8Result
{
    std::int32_t attack = 0;
    std::int32_t health = 0;
    std::int32_t gold = 0;
    std::int32_t damage = 0;
    std::int32_t damageTargets = 0;
    std::int32_t upgradeCostDelta = 0;
    std::int32_t bananaCards = 0;
    bool divineShield = false;
    bool temporaryUntilNextTurn = false;
    bool rebornDeathrattle = false;
    bool noTypeOnly = false;
    bool freeRefresh = false;
    bool allEnemyMinions = false;
    bool summonMurlocDeathrattle = false;
};

//! Resolve deterministic lifecycle hooks.  The caller applies returned
//! deltas atomically at the corresponding game boundary.
constexpr void ResolveSeason14HeroPowerBatch8Event(
    std::int32_t dbfID, Season14HeroPowerBatch8Event event,
    Season14HeroPowerBatch8State& state,
    Season14HeroPowerBatch8Result& result) noexcept
{
    result = {};
    if (FindSeason14HeroPowerBehaviorBatch8(dbfID) == nullptr) return;
    if (event == Season14HeroPowerBatch8Event::BEGIN_TURN)
    {
        ++state.turnNumber;
        if (dbfID == 59815) result.bananaCards = 0;
        return;
    }
}

//! Resolve deterministic target-free portions of an activation.  A false
//! return means the power is passive, exhausted, or needs a caller-owned
//! target/modal; such actions must fail closed in the bridge.
constexpr bool ResolveSeason14HeroPowerBatch8Activation(
    std::int32_t dbfID, Season14HeroPowerBatch8State& state,
    Season14HeroPowerBatch8Result& result) noexcept
{
    result = {};
    (void)state;
    switch (dbfID)
    {
        case 57957:
        case 58017:
        case 58527:
        case 59862:
            return true;
        case 57962: result.attack = 10; result.temporaryUntilNextTurn = true; return true;
        case 60376: result.attack = 1; result.health = 1; result.noTypeOnly = true; return true;
        case 59815: result.bananaCards = 2; return true;
        default: return false;
    }
}

struct Season14HeroPowerBatch8PassiveModifiers
{
    std::int32_t upgradeCostDelta = 0;
};

constexpr Season14HeroPowerBatch8PassiveModifiers
Season14HeroPowerBatch8Modifiers(std::int32_t dbfID) noexcept
{
    // Skilled Bartender: reduce the Tavern upgrade cost by one.
    return dbfID == 57561 ? Season14HeroPowerBatch8PassiveModifiers{-1}
                          : Season14HeroPowerBatch8PassiveModifiers{};
}

//! Return start-of-combat deterministic effects.  Damage recipient choice is
//! intentionally left to the combat resolver.
constexpr bool ResolveSeason14HeroPowerBatch8CombatStart(
    std::int32_t dbfID, Season14HeroPowerBatch8Result& result) noexcept
{
    result = {};
    switch (dbfID)
    {
        case 58017: result.damage = 8; result.damageTargets = 2; return true;
        case 58527: result.damage = 4; result.damageTargets = 2; return true;
        case 59862: result.damage = 1; result.damageTargets = -1; result.allEnemyMinions = true; return true;
        case 57957: result.summonMurlocDeathrattle = true; return true;
        default: return false;
    }
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH8_HPP
