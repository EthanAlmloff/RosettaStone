// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_GIANT_SPELLCRAFT_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_GIANT_SPELLCRAFT_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
struct GiantSpellcraftDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    std::string_view linkedMinion;
    std::int32_t linkedMinionDbfID;
};

inline constexpr std::array<GiantSpellcraftDefinition, 4>
    GIANT_SPELLCRAFT_BEHAVIORS = {{
        {"BG34_Giant_035", 126517, "BG34_Giant_035t", 126518},
        {"BG34_Giant_035_G", 127386, "BG34_Giant_035t_G", 128719},
        {"BG34_Giant_035t", 126518, "", 0},
        {"BG34_Giant_035t_G", 128719, "", 0},
    }};

constexpr const GiantSpellcraftDefinition* FindGiantSpellcraft(
    std::int32_t dbfID) noexcept
{
    for (const auto& definition : GIANT_SPELLCRAFT_BEHAVIORS)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}
}
#endif
