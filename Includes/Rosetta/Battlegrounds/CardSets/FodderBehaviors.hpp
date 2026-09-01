// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_FODDER_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_FODDER_BEHAVIORS_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Pinned Season 14 Fodder family.  The token is created during Refresh;
//! Defiler entries arm that same refresh lifecycle at recruit end.
struct FodderBehaviorDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    enum class Kind : std::uint8_t { TOKEN, DEFILER } kind;
    std::int32_t attack;
    std::int32_t health;
    std::int32_t foddersPerRefresh;
};

inline constexpr std::array<FodderBehaviorDefinition, 4>
    FODDER_BEHAVIORS = {{
        {"BG35_150t", 130084, FodderBehaviorDefinition::Kind::TOKEN, 2, 2, 0},
        {"BG35_150t_G", 130085, FodderBehaviorDefinition::Kind::TOKEN, 4, 4, 0},
        {"BG35_151", 130076, FodderBehaviorDefinition::Kind::DEFILER, 5, 6, 1},
        {"BG35_151_G", 130081, FodderBehaviorDefinition::Kind::DEFILER, 10, 12, 2},
    }};

constexpr const FodderBehaviorDefinition* FindFodderBehavior(
    std::string_view id) noexcept
{
    for (const auto& behavior : FODDER_BEHAVIORS)
        if (behavior.id == id) return &behavior;
    return nullptr;
}

constexpr const FodderBehaviorDefinition* FindFodderBehaviorByDbfID(
    std::int32_t dbfID) noexcept
{
    for (const auto& behavior : FODDER_BEHAVIORS)
        if (behavior.dbfID == dbfID) return &behavior;
    return nullptr;
}
}

#endif
