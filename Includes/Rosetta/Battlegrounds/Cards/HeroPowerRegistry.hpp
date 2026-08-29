// Copyright (c) 2026 HSBGML contributors
//
// Patch 36.4 hero/power metadata is loaded from HearthstoneJSON by CardLoader.
// This header contains only the fixed registry contract needed to assemble and
// validate that metadata. It intentionally does not register behavior in
// CardDefs: effects are not implemented yet and must fail loudly when called.

#ifndef ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_REGISTRY_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_REGISTRY_HPP

#include <array>
#include <string_view>

namespace RosettaStone::Battlegrounds::HeroPowerRegistry
{
inline constexpr std::size_t EXPECTED_HERO_COUNT = 116;
inline constexpr std::size_t EXPECTED_POWER_COUNT = 120;

// Powers in the pinned inventory that are not referenced by an active hero.
inline constexpr std::array<std::string_view, 4>
    GENERATED_REPLACEMENT_POWER_IDS = {
        "BG22_HERO_007p2",
        "TB_BaconShop_HP_036t",
        "TB_BaconShop_HP_065t2",
        "TB_BaconShop_HP_087t",
    };

inline constexpr bool IsGeneratedReplacement(std::string_view id) noexcept
{
    for (const auto candidate : GENERATED_REPLACEMENT_POWER_IDS)
    {
        if (candidate == id)
        {
            return true;
        }
    }
    return false;
}
}  // namespace RosettaStone::Battlegrounds::HeroPowerRegistry

#endif  // ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_REGISTRY_HPP
