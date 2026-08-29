// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! The executable behavior family for a pinned Season 14 hero power.
//!
//! This is intentionally a small, explicit registry.  It is not inferred
//! from card text: an ID may enter the supported pool only after its family
//! has a corresponding simulator hook and tests.
enum class Season14HeroPowerKind : std::uint8_t
{
    STARTING_HEALTH,
    TAVERN_MINION_AURA,
    ECONOMY_COST_AURA,
    GOLD_SCALING,
    FREE_REFRESH,
    BUY_PIRATE_GOLD,
    TAVERN_SPELL_AURA,
    MAX_GOLD,
};

struct Season14HeroPowerDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerKind kind;
    std::int32_t cost;
    bool passive;
};

//! Exact Patch 36.4 behavior batch (eight distinct reusable families).
inline constexpr std::array<Season14HeroPowerDefinition, 8>
    SEASON14_HERO_POWER_BEHAVIORS = {{
        {"TB_BaconShop_HP_035", 59399,
         Season14HeroPowerKind::STARTING_HEALTH, 0, true},
        {"BG20_HERO_102p", 71455,
         Season14HeroPowerKind::TAVERN_MINION_AURA, 0, true},
        {"TB_BaconShop_HP_054", 60405,
         Season14HeroPowerKind::ECONOMY_COST_AURA, 0, true},
        {"TB_BaconShop_HP_076", 62269,
         Season14HeroPowerKind::GOLD_SCALING, 0, false},
        {"TB_BaconShop_HP_063", 61491,
         Season14HeroPowerKind::FREE_REFRESH, 0, true},
        {"BG26_HERO_101p", 101132,
         Season14HeroPowerKind::BUY_PIRATE_GOLD, 0, true},
        {"TB_BaconShop_HP_085t", 122960,
         Season14HeroPowerKind::TAVERN_SPELL_AURA, 0, true},
        {"BG32_HERO_001p", 116921,
         Season14HeroPowerKind::MAX_GOLD, 3, false},
    }};

constexpr const Season14HeroPowerDefinition* FindSeason14HeroPowerBehavior(
    std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerDefinition* FindSeason14HeroPowerBehavior(
    std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr bool HasSeason14HeroPowerBehavior(std::int32_t dbfID) noexcept
{
    return FindSeason14HeroPowerBehavior(dbfID) != nullptr;
}

//! Data-only effects produced by a no-target activation.
struct Season14HeroPowerActivation
{
    std::int32_t goldDelta = 0;
    std::int32_t maxGoldDelta = 0;
    bool consumesTurnUse = true;
};

//! Resolve the two no-target active powers in this batch.
//!
//! `turnNumber` is one-based: the first recruit turn returns +2 for Piggy
//! Bank, the second returns +3, and so on.  Passive and target-dependent
//! powers return false so callers cannot accidentally claim an incomplete
//! implementation.
constexpr bool ResolveSeason14HeroPowerActivation(
    std::int32_t dbfID, std::int32_t turnNumber,
    Season14HeroPowerActivation& result) noexcept
{
    result = {};
    if (dbfID == 62269) // TB_BaconShop_HP_076, Piggy Bank
    {
        result.goldDelta = 1 + (turnNumber > 0 ? turnNumber : 1);
        return true;
    }
    if (dbfID == 116921) // BG32_HERO_001p, Wisdom of Ancients
    {
        result.maxGoldDelta = 1;
        return true;
    }
    return false;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
