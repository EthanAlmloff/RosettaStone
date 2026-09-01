// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_FISHBAIT_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_FISHBAIT_BEHAVIORS_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
enum class FishbaitSellKind : std::uint8_t { AIR_BALLER, SNARKY_SHARK };

struct FishbaitSellDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    FishbaitSellKind kind;
    std::int32_t stat;
    std::int32_t generatedFishbaitDbfID;
};

//! Generated Fishbait entities retain their own typed deathrattle contract.
//! They are not ordinary CardDefs because the Shark lifecycle may resolve
//! them immediately when the left-most Beast attacks.
struct FishbaitTokenDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    std::int32_t killerStat;
};

inline constexpr std::array<FishbaitTokenDefinition, 2>
    FISHBAIT_TOKEN_BEHAVIORS = {{
        {"BG36_205", 132802, 5},
        {"BG36_205_G", 132803, 10},
    }};

constexpr const FishbaitTokenDefinition* FindFishbaitTokenBehavior(
    std::int32_t dbfID) noexcept
{
    for (const auto& definition : FISHBAIT_TOKEN_BEHAVIORS)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

inline constexpr std::array<FishbaitSellDefinition, 4>
    FISHBAIT_SELL_BEHAVIORS = {{
        {"BG36_181", 133455, FishbaitSellKind::AIR_BALLER, 2, 0},
        {"BG36_181_G", 133456, FishbaitSellKind::AIR_BALLER, 4, 0},
        {"BG36_206", 132804, FishbaitSellKind::SNARKY_SHARK, 0, 132802},
        {"BG36_206_G", 132805, FishbaitSellKind::SNARKY_SHARK, 0, 132803},
    }};

constexpr const FishbaitSellDefinition* FindFishbaitSellBehavior(
    std::int32_t dbfID) noexcept
{
    for (const auto& definition : FISHBAIT_SELL_BEHAVIORS)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_FISHBAIT_BEHAVIORS_HPP
