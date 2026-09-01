// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_DETECTIVE_FOR_HIRE_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DETECTIVE_FOR_HIRE_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>
namespace RosettaStone::Battlegrounds {
struct DetectiveForHireDefinition { std::string_view id; std::int32_t dbfID; };
inline constexpr std::array DETECTIVE_FOR_HIRE_BEHAVIORS = {
 DetectiveForHireDefinition{"BG23_HERO_303p2", 90403},
 DetectiveForHireDefinition{"BG23_HERO_303pt", 92974},
 DetectiveForHireDefinition{"BG23_HERO_303_Buddy", 98656},
 DetectiveForHireDefinition{"BG23_HERO_303_Buddy_G", 98659},
};
}
#endif
