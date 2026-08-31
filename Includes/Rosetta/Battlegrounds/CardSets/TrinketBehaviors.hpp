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
                           SHOP_STATS_AND_EXTRA_SLOT,
                           START_TURN_GOLD_PER_MINION_TYPE, IMMEDIATE_GOLD,
                           ACQUIRE_RANDOM_MINIONS,
                           START_TURN_RANDOM_MINIONS,
                           ACQUIRE_FIXED_CARD,
                           END_TURN_MAX_GOLD,
                           END_TURN_GOLDEN_STATS,
                           AFTER_PLAY_HAND_BUFF,
                           AFTER_PLAY_ELEMENTAL_SHOP_BUFF,
                           AFTER_TAVERN_SPELL_SHOP_BUFF,
                           AFTER_TAVERN_SPELL_RACE_BUFF,
                           TAVERN_SPELL_STATS,
                           AFTER_BUY_RANDOM_FRIENDLY_BUFF,
                           END_TURN_DIVINE_SHIELD_ATTACK,
                           AFTER_PLAY_CARD_RANDOM_RACE_BUFF,
                           STATIC_RACE_STATS,
                           AFTER_REBORN_STATS,
                           DUPLICATE_DRAGON_BATTLECRY,
                           FIRST_MINION_DIVINE_SHIELD,
                           BATTLECRY_BUY_DISCOUNT };
                           
                           
                           
struct TrinketBehavior
{
    TrinketEffect effect = TrinketEffect::NONE;
    int attack = 0;
    int health = 0;
    int value = 0;
    Race race = Race::INVALID;
    int tier = 0;
    int amount = 0;
    bool repeatAtStartTurn = false;
    bool battlecryOnly = false;
    bool magneticOnly = false;
    // Canonical generated card for fixed-card acquisition effects.
    std::string_view cardID{};
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
