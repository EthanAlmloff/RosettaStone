// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_SEWER_RAT_TOKEN_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEWER_RAT_TOKEN_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>
namespace RosettaStone::Battlegrounds {
struct SewerRatTokenDefinition { std::string_view id; std::int32_t dbfID; std::int32_t attack; std::int32_t health; };
inline constexpr std::array SEWER_RAT_TOKEN_BEHAVIORS = {
    SewerRatTokenDefinition{"BG19_010", 70790, 3, 2},
    SewerRatTokenDefinition{"BG19_010_G", 70801, 6, 4},
    SewerRatTokenDefinition{"BG19_010t", 70791, 2, 3},
    SewerRatTokenDefinition{"BG19_010_Gt", 70802, 4, 6},
};
constexpr const SewerRatTokenDefinition* FindSewerRatToken(std::int32_t dbfID) noexcept {
    for (const auto& d : SEWER_RAT_TOKEN_BEHAVIORS) if (d.dbfID == dbfID) return &d;
    return nullptr;
}
}
#endif
