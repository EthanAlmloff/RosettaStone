// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_HELPFUL_CARD_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HELPFUL_CARD_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>
namespace RosettaStone::Battlegrounds {
struct HelpfulCardDefinition { std::string_view id; std::int32_t dbfID; };
// Patch 36.4 does not publish the server-side "helpful card" candidate pool.
// Keep this empty rather than treating arbitrary Discover cards as equivalent.
inline constexpr std::array<HelpfulCardDefinition, 0> HELPFUL_CARD_BEHAVIORS{};
}
#endif
