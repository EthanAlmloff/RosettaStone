// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH6_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH6_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
enum class Season14HeroPowerBatch6Kind : std::uint8_t
{
    VOID_POWER,
};

struct Season14HeroPowerBatch6Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch6Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch6Definition, 1>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH6 = {{
        {"BG36_HERO_101p", 132581,
         Season14HeroPowerBatch6Kind::VOID_POWER, 0, false},
    }};

constexpr const Season14HeroPowerBatch6Definition*
FindSeason14HeroPowerBehaviorBatch6(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH6)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

constexpr const Season14HeroPowerBatch6Definition*
FindSeason14HeroPowerBehaviorBatch6(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH6)
        if (definition.id == id) return &definition;
    return nullptr;
}

struct Season14HeroPowerBatch6State
{
    std::int32_t turnNumber = 0;
    bool discoverReady = false;
    bool discoverOffered = false;
};

constexpr bool IsVoidPower(std::int32_t dbfID) noexcept { return dbfID == 132581; }

//! Void Power unlocks its single Tier-5 Dark Gift Discover on recruit turn 7.
constexpr bool ResolveVoidPowerBeginTurn(
    std::int32_t dbfID, Season14HeroPowerBatch6State& state) noexcept
{
    if (!IsVoidPower(dbfID) || state.discoverOffered) return false;
    ++state.turnNumber;
    if (state.turnNumber < 7) return false;
    state.discoverReady = true;
    return true;
}

constexpr bool ConsumeVoidPowerDiscover(
    Season14HeroPowerBatch6State& state) noexcept
{
    if (!state.discoverReady || state.discoverOffered) return false;
    state.discoverReady = false;
    state.discoverOffered = true;
    return true;
}

//! Rolls back the reservation when the public Discover cannot be created.
constexpr void RestoreVoidPowerDiscoverReady(
    Season14HeroPowerBatch6State& state) noexcept
{
    if (state.discoverOffered) return;
    state.discoverReady = true;
}
}  // namespace RosettaStone::Battlegrounds
#endif
