// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH7_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH7_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Delayed Timewarp visits.  The visit is a replayable Choice/Discover modal;
//! this registry owns only the authoritative trigger turn.
//! Offering generation remains an explicit caller responsibility and must fail closed.
//! A pinned Timewarp pool is required before constructing the public modal.
enum class Season14HeroPowerBatch7Kind : std::uint8_t
{
    MINOR_TIMEWARP,
    MAJOR_TIMEWARP,
};

struct Season14HeroPowerBatch7Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch7Kind kind;
    std::int32_t triggerTurn;
};

inline constexpr std::array<Season14HeroPowerBatch7Definition, 2>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH7 = {{
        {"BG34_HERO_004p", 129174,
         Season14HeroPowerBatch7Kind::MINOR_TIMEWARP, 5},
        {"BG34_HERO_000p", 127697,
         Season14HeroPowerBatch7Kind::MAJOR_TIMEWARP, 8},
    }};

constexpr const Season14HeroPowerBatch7Definition*
FindSeason14HeroPowerBehaviorBatch7(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH7)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

constexpr const Season14HeroPowerBatch7Definition*
FindSeason14HeroPowerBehaviorBatch7(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH7)
        if (definition.id == id) return &definition;
    return nullptr;
}

struct Season14HeroPowerBatch7State
{
    std::int32_t recruitTurnNumber = 0;
    bool visitReady = false;
    bool visitConsumed = false;
};

constexpr bool ResolveSeason14HeroPowerBatch7BeginTurn(
    std::int32_t dbfID, Season14HeroPowerBatch7State& state) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehaviorBatch7(dbfID);
    if (definition == nullptr || state.visitConsumed || state.visitReady)
        return false;
    ++state.recruitTurnNumber;
    if (state.recruitTurnNumber != definition->triggerTurn) return false;
    state.visitReady = true;
    return true;
}

//! Reserve the visit before constructing a public modal.  Callers must call
//! Restore... if the pinned offering pool cannot be built.
constexpr bool ConsumeSeason14HeroPowerBatch7Visit(
    Season14HeroPowerBatch7State& state) noexcept
{
    if (!state.visitReady || state.visitConsumed) return false;
    state.visitReady = false;
    state.visitConsumed = true;
    return true;
}

constexpr void RestoreSeason14HeroPowerBatch7Visit(
    Season14HeroPowerBatch7State& state) noexcept
{
    if (!state.visitConsumed) state.visitReady = true;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH7_HPP
