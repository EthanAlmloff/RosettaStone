// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_TRINKET_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TRINKET_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
enum class TrinketEffect { NONE, SHOP_STATS, EXTRA_SHOP_SLOT,
                           HIGHER_TIER_REFRESH, MAX_GOLD, GOLD_AND_MAX_GOLD,
                           SHOP_STATS_AND_EXTRA_SLOT };
struct TrinketBehavior
{
    TrinketEffect effect = TrinketEffect::NONE;
    int attack = 0;
    int health = 0;
    int value = 0;
};

//! Exact, executable subset of Patch 36.4 passive Trinkets.
TrinketBehavior FindTrinketBehavior(std::string_view id) noexcept;
class TrinketBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds
#endif
