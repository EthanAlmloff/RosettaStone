// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Explicit, fully executable families from the third Season 14 batch.
//!
//! This registry is intentionally small.  An ID is added here only when the
//! simulator has a complete transition for it; metadata-only powers remain
//! absent and therefore fail closed in the bridge.
enum class Season14HeroPowerBatch3Kind : std::uint8_t
{
    COMBAT_KILL_ATTACK,
    CONVICTION_IMPROVEMENT,
};

struct Season14HeroPowerBatch3Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch3Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch3Definition, 2>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH3 = {{
        {"BG20_HERO_100p", 80229,
         Season14HeroPowerBatch3Kind::COMBAT_KILL_ATTACK, 0, true},
        {"BG21_HERO_000p", 73941,
         Season14HeroPowerBatch3Kind::CONVICTION_IMPROVEMENT, 0, false},
    }};

constexpr const Season14HeroPowerBatch3Definition*
FindSeason14HeroPowerBehaviorBatch3(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH3)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerBatch3Definition*
FindSeason14HeroPowerBehaviorBatch3(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH3)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

//! Result of a target-free Batch-3 activation.
//!
//! Choice-based powers do not use this target-free result. It remains the
//! narrow extension point for stateless Batch-3 activations.
struct Season14HeroPowerBatch3Activation
{
    std::int32_t attack = 0;
    std::int32_t health = 0;
    //! Number of random minions to choose.  Zero means all friendly minions.
    std::int32_t randomCount = 0;
};

//! Resolve a complete target-free Batch-3 activation.
//!
//! Choice-based Conviction is resolved through Player's replayable modal;
//! other unregistered families return false.
constexpr bool ResolveSeason14HeroPowerBatch3Activation(
    std::int32_t dbfID, std::int32_t currentTier,
    Season14HeroPowerBatch3Activation& result) noexcept
{
    result = {};
    (void)currentTier;
    return false;
}

//! Return the permanent attack bonus awarded by Glory of Combat after a
//! friendly minion kills an enemy.  Unsupported powers return zero.
constexpr std::int32_t Season14HeroPowerBatch3CombatKillAttack(
    std::int32_t dbfID) noexcept
{
    return dbfID == 80229 ? 1 : 0;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP
