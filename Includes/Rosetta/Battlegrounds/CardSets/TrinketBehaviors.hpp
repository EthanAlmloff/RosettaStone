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
                           ACQUIRE_RANDOM_FRIENDLY_COPY,
                           START_TURN_RANDOM_MINIONS,
                           AFTER_FRIENDLY_DEATH_RANDOM_MINION,
                           AFTER_SELL_RANDOM_MINION,
                           ACQUIRE_FIXED_CARD,
                           ACQUIRE_FIXED_GLOWSCALE,
                           ACQUIRE_FIXED_LIONFISH,
                           END_TURN_MAX_GOLD,
                           END_TURN_GOLDEN_STATS,
                           AFTER_PLAY_HAND_BUFF,
                           AFTER_PLAY_ELEMENTAL_SHOP_BUFF,
                           AFTER_TAVERN_SPELL_SHOP_BUFF,
                           AFTER_TAVERN_SPELL_RACE_BUFF,
                           TAVERN_SPELL_STATS,
                           TAVERN_SPELL_GROWING_STATS,
                           TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST,
                           AFTER_BUY_RANDOM_FRIENDLY_BUFF,
                           END_TURN_DIVINE_SHIELD_ATTACK,
                           AFTER_PLAY_CARD_RANDOM_RACE_BUFF,
                           STATIC_RACE_STATS,
                           AFTER_REBORN_STATS,
                           DUPLICATE_DRAGON_BATTLECRY,
                           FIRST_MINION_DIVINE_SHIELD,
                           BATTLECRY_BUY_DISCOUNT,
                           REFRESH_SHOP_STATS,
                           HERO_DAMAGE_SHOP_STATS,
                           STATIC_MINION_STATS,
                           BLOOD_GEM_BONUS,
                           START_COMBAT_MINION_STATS,
                           STATIC_TIER_MINION_STATS,
                           STATIC_FODDER_SHOP_STATS,
                           TAVERN_STATS_PER_SOLD,
                           UPGRADE_COST_DISCOUNT,
                           CONDUCTOR_DISCARD_BLOOD_GEM,
                           SHIP_IN_A_BOTTLE,
                           BRONZEBEARD_PORTRAIT,
                           NEXT_TAVERN_SPELL_DISCOUNT,
                           STAT_TAVERN_SPELL_DISCOUNT,
                           FREE_TAVERN_SPELL_USES,
                           START_TURN_GOLD_DAMAGE,
                           REFRESH_TEMP_SHOP_STATS,
                           AVENGE_MINION_STATS,
                           TAVERN_SPELL_TEMP_STATS_AFTER_DAMAGE,
                           PIRATE_ATTACK_GOLD,
                           ATTACKING_MINION_STATS,
                           START_COMBAT_HEALTH_FROM_ATTACK,
                           AVENGE_TAVERN_SPELL_ATTACK,
                           REFRESH_EXTRA_SHOP_SLOTS,
                           SPELL_COUNT_MINION_ATTACK,
                           END_TURN_UNDEAD_ATTACK,
                           REACH_TIER_GOLD,
                           DELAYED_GOLD,
                           SPELL_CAST_MINION_STATS,
                           // Combat-only trigger: the next five summoned
                           // friendly minions each receive Divine Shield.
                           SUMMON_DIVINE_SHIELD,
                           START_COMBAT_NAGA_SPELLCRAFT,
                           AFTER_PLAY_NAGA_SPELLCRAFT,
                           SUMMON_MECH_RANDOM_DIVINE_SHIELD,
                           START_COMBAT_QUILBOAR_BLOOD_GEMS,
                           SUMMON_BEAST_DOUBLE_ATTACK,
                           SUMMON_BEAST_STATS,
                           SUMMON_BEAST_RANDOM_MINION,
                           START_COMBAT_ELEMENTAL_FROSTLING,
                           START_COMBAT_BEAST_SCALING,
                           START_COMBAT_QUILBOAR_BLOOD_GOLEM,
                           START_COMBAT_EDGE_SHIELDS,
                           START_COMBAT_LEFT_COPY,
                           START_COMBAT_UNDEAD_EDGE_REBORN,
                           START_COMBAT_TRIGGER_DEATHRATTLES,
                           START_COMBAT_HIGHEST_HAND_MINION,
                           AFTER_DEATHRATTLE_RIGHTMOST_STATS,
                           START_COMBAT_NEUTRAL_TRIPLE,
                           START_COMBAT_DRAGON_MAX_ATTACK,
                           START_COMBAT_LEFTMOST_HAND_STATS,
                           START_COMBAT_LOWEST_ATTACK_DOUBLE,
                           START_COMBAT_LEFT_BEAST_SHIELDS,
                           START_COMBAT_HIGHEST_TIER_DRAGON_GOLDEN,
                           START_COMBAT_THREE_BLOOD_GEMS,
                           START_COMBAT_TYPE_STATS,
                           START_COMBAT_NAGA_HEALTH,
                           START_COMBAT_RANDOM_PIRATE_SHIELDS,
                           START_COMBAT_MURLOC_MAX_ATTACK,
                           START_COMBAT_RALLY_SHIELDS,
                           START_COMBAT_DRAGON_SHIELDS,
                           START_COMBAT_NAGA_DOUBLE_STATS,
                           AVENGE_BLOOD_GEM_BONUS,
                           AVENGE_RANDOM_UNDEAD_REBORN,
                           FIRST_DEATH_MAX_STATS_RANDOM,
                           AVENGE_RANDOM_MAGNETIC,
                           AFTER_TWO_ATTACKS_QUILBOAR_GEM,
                           SPELL_COUNT_RANDOM_NAGA,
                           SPELL_COUNT_GOLD_ON_MINION,
                           TAVERN_SPELL_NO_TYPE_STATS,
                           BLOOD_GEM_DIVINE_SHIELD,
                           ATTACKING_DRAGON_DIVINE_SHIELD,
                           ATTACKING_BEAST_SCALING,
                           END_TURN_BLOOD_GEMS_PER_TYPE,
                           END_TURN_MURLOC_STATS,
                           AFTER_MAGNETIC_MECH_REPAIR,
                           END_TURN_MINION_STATS,
                           RALLY_ATTACK_FREE_REFRESH,
                           BATTLECRY_EDGE_STATS,
                           END_TURN_LEFT_DEATHRATTLES,
                           END_TURN_RALLY_TRIGGERS,
                           AFTER_SPELLCRAFT_DIVINE_SHIELD_STATS,
                           AFTER_BEAST_ATTACK_STATS,
                           HERO_POWER_TWICE,
                           SPELLCRAFT_TRANSFORM_HIGHER_TIER,
                           PRECIOUS_PEARL_SPELLCRAFT,
                           BOOK_OF_MEDIVH_DISCOVER,
                           AZEROTH_MODEL_GLOBE,
                           END_TURN_RANDOM_TYPE_MINIONS,
                           ACQUIRE_LAST_OPPONENT_COPY,
                           FIRST_SPELL_REPEAT,
                           SPELLCRAFT_REPEAT,
                           AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL,
                           END_TURN_FIXED_CARD,
                           AFTER_BUY_BATTLECRY_MINION,
                           START_COMBAT_GOLDEN_FISH,
                           SPEND_GOLD_PIRATE_STATS };

//! Extra text carried by a fixed-portrait Trinket.  This is deliberately
//! separate from TrinketEffect: the fixed card can be executable while the
//! portrait's account-wide modifier is not yet represented by the simulator.
//! Keeping the distinction in the descriptor prevents an acquisition-only
//! registration from receiving full executable-coverage credit.
enum class PortraitEffect {
    NONE,
    LIVING_AZERITE_ELEMENTAL_STATS,
    BELCHER_VENOMOUS_LOSS_STATS,
    SURPRISE_MORE_ELEMENTALS,
    KABOOM_BOT_DEATHRATTLE_DAMAGE,
    WHELP_SMUGGLER_STATS_AND_DRAGON,
    ZESTY_SHAKER_EXTRA_COPY,
    FELBLOOD_BOTH_STATS,
    LIGHTFANG_ALL_TYPES,
    BRISTLEBACH_ALL_MINIONS,
    BASSGILL_SUMMON_DIVINE_SHIELD,
    SELFLESS_BATTLECRY,
    GROUNDBREAKER_ADJACENT_STATS
};
// End-of-recruit persistent race aura.
// Effects whose trigger is a successful recruit-phase refresh or self-damage
// are deliberately separate from static auras: their counters must survive
// Tavern replacement and be replayable from Season14State.
                           
                           
                           
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
    //! Portrait-only modifier, if the Trinket is one of the fixed portraits.
    PortraitEffect portraitEffect = PortraitEffect::NONE;
    //! True only when the modifier above is fully applied by the simulator.
    bool portraitExecutable = false;
    // Optional count for a recurring fixed-card grant when it differs from
    // the acquisition count (for example Essence of Dreams: 2 now, 1 each
    // subsequent recruit start).
    int startTurnAmount = 0;
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
