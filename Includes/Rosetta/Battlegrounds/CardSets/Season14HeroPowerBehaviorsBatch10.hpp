// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH10_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH10_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Final pinned Patch 36.4 hero-power records not covered by the earlier
//! reusable families.  Execution remains in the normal bridge/Player
//! lifecycle; this table supplies stable identity, payment, and whether the
//! power is exposed as a recruit action.
enum class Season14HeroPowerBatch10Kind : std::uint8_t
{
    PUZZLE_BOX,
    A_TALE_OF_KINGS,
    PROCRASTINATE,
    PIRATE_PARRRRTY,
    TRASH_FOR_TREASURE,
    COME_ONE_COME_ALL,
    THREE_WISHES,
    RUNE_OF_DAMNATION,
};

struct Season14HeroPowerBatch10Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch10Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch10Definition, 8>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH10 = {{
        // Puzzle Box resolves one target-free Tavern spell at each recruit
        // boundary after turn three; it is intentionally not a free action.
        {"TB_BaconShop_HP_039t", 122958,
         Season14HeroPowerBatch10Kind::PUZZLE_BOX, 0, true},
        {"TB_BaconShop_HP_041", 63127,
         Season14HeroPowerBatch10Kind::A_TALE_OF_KINGS, 2, false},
        {"TB_BaconShop_HP_044", 59891,
         Season14HeroPowerBatch10Kind::PROCRASTINATE, 0, false},
        {"TB_BaconShop_HP_072", 62243,
         Season14HeroPowerBatch10Kind::PIRATE_PARRRRTY, 3, false},
        {"TB_BaconShop_HP_075", 62267,
         Season14HeroPowerBatch10Kind::TRASH_FOR_TREASURE, 0, false},
        {"TB_BaconShop_HP_101", 64481,
         Season14HeroPowerBatch10Kind::COME_ONE_COME_ALL, 0, false},
        {"TB_BaconShop_HP_102", 64486,
         Season14HeroPowerBatch10Kind::THREE_WISHES, 3, false},
        {"TB_BaconShop_HP_702t", 122959,
         Season14HeroPowerBatch10Kind::RUNE_OF_DAMNATION, 1, false},
    }};

constexpr const Season14HeroPowerBatch10Definition*
FindSeason14HeroPowerBehaviorBatch10(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH10)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

constexpr const Season14HeroPowerBatch10Definition*
FindSeason14HeroPowerBehaviorBatch10(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH10)
        if (definition.id == id) return &definition;
    return nullptr;
}

constexpr bool HasSeason14HeroPowerBehaviorBatch10(std::int32_t dbfID) noexcept
{
    return FindSeason14HeroPowerBehaviorBatch10(dbfID) != nullptr;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH10_HPP
