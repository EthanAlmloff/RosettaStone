// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_TAVERN_SPELL_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TAVERN_SPELL_BEHAVIORS_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>

#include <cstddef>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! The small set of effect shapes currently implemented for Tavern spells.
//!
//! These are simulator effects, rather than metadata classifications.  A
//! spell is only exposed by Player::CanPlaySpell when this lookup returns a
//! non-NONE behavior.
enum class TavernSpellEffect
{
    NONE,
    ALL_STATS,
    ALL_STATS_AND_GOLDEN,
    LEFTMOST_STATS,
    DIVINE_SHIELD_ATTACK,
    ALL_AND_RACE,
    ALL_RACE_AND_DIVINE_SHIELD,
    RANDOM_STATS,
    MENAGERIE_STATS,
    ONE_PER_RACE_STATS,
    SHOP_STATS,
    TARGET_STATS,
    SET_TARGET_STATS,
    TARGET_AND_RACE,
    TARGET_STATS_REPEAT,
    TARGET_STATS_AND_TAUNT,
    TARGET_DIVINE_SHIELD,
    TARGET_STATS_TOGGLE_TAUNT,
    SET_PLAYER_ARMOR,
    NEXT_TURN_GOLD,
    INCREASE_MAX_GOLD,
    FREE_REFRESHES,
    SHOP_STATS_PERSISTENT,
    SPELL_COSTS_HEALTH,
};

struct TavernSpellBehavior
{
    int gold = -1;
    int attack = 0;
    int health = 0;
    TavernSpellEffect effect = TavernSpellEffect::NONE;
    Race race = Race::INVALID;
    int randomCount = 0;
    //! Secondary scalar for economy/player-state effects.  Keeping this
    //! separate from attack/health makes table entries self-describing and
    //! prevents a future executor from confusing a gold value with a stat.
    int value = 0;
};

//! Menagerie Tableware applies its base buff once, then repeats it once for
//! each distinct friendly minion type.  Keep the count in the shared schema so
//! the executor and focused tests cannot silently disagree about the initial
//! cast versus the repeats.
constexpr std::size_t MenagerieTablewareRepeatCount(
    std::size_t distinctFriendlyTypes) noexcept
{
    return distinctFriendlyTypes + 1;
}

//! Returns the exact behavior implemented for a pinned Patch 36.4 spell ID.
//!
//! Keep this table conservative: metadata and card text do not establish
//! simulator support.  Every entry is covered by a focused behavior test.
inline TavernSpellBehavior FindTavernSpellBehavior(std::string_view id)
{
    if (id == "BG28_168") // Shiny Ring: Give your minions +1/+1.
    {
        return { 0, 1, 1, TavernSpellEffect::ALL_STATS };
    }
    if (id == "BG28_169") // Azerite Empowerment: +2/+2 twice.
    {
        return { 0, 4, 4, TavernSpellEffect::ALL_STATS };
    }
    if (id == "BG33_813") // Selfish Bounty: left-most +6/+6.
    {
        return { 0, 6, 6, TavernSpellEffect::LEFTMOST_STATS };
    }
    if (id == "BG33_817") // Sanctify: Divine Shield minions +6 Attack.
    {
        return { 0, 6, 0, TavernSpellEffect::DIVINE_SHIELD_ATTACK };
    }
    if (id == "BG35_922") // Queen's Command: all +2/+2, Naga another +2/+2.
    {
        return { 0, 2, 2, TavernSpellEffect::ALL_AND_RACE, Race::NAGA };
    }
    if (id == "BG36_246") // Mighty Dragonbreath: dragons and shields repeat.
    {
        return { 0, 2, 1, TavernSpellEffect::ALL_RACE_AND_DIVINE_SHIELD,
                 Race::DRAGON };
    }
    if (id == "BG28_810") // Tavern Coin: Gain 1 Gold.
    {
        return { 1 };
    }
    if (id == "BG33_815") // Wealthy Bounty: Gain 2 Gold.
    {
        return { 2 };
    }

    // Patch 36.4 batch: deterministic board/shop buffs and seeded random
    // friendly-minion selections.  The random choices are made by the
    // simulator's seeded engine at resolution time.
    if (id == "BG34_272") // Menagerie Tableware: +3/+3 per distinct type.
    {
        return { 0, 3, 3, TavernSpellEffect::MENAGERIE_STATS };
    }
    if (id == "BG34_990") // Wave of Gold: all +3/+2, Golden another +3/+2.
    {
        return { 0, 3, 2, TavernSpellEffect::ALL_STATS_AND_GOLDEN };
    }
    if (id == "BG28_966") // Them Apples: Tavern minions +1/+2.
    {
        return { 0, 1, 2, TavernSpellEffect::SHOP_STATS };
    }
    if (id == "BG33_811") // Healthy Bounty: four random friendly +4 Health.
    {
        return { 0, 0, 4, TavernSpellEffect::RANDOM_STATS, Race::INVALID, 4 };
    }
    if (id == "BG33_812") // Hostile Bounty: four random friendly +4 Attack.
    {
        return { 0, 4, 0, TavernSpellEffect::RANDOM_STATS, Race::INVALID, 4 };
    }
    if (id == "BG35_951") // Might of Stormwind: four random +1/+2.
    {
        return { 0, 1, 2, TavernSpellEffect::RANDOM_STATS, Race::INVALID, 4 };
    }

    // Patch 36.4 bounded target/stat batch. These effects use the existing
    // minion stat and race primitives; target selection is supplied by the
    // structured TavernSpell action rather than inferred from mutable state.
    if (id == "BG28_838") // Perfect Vision: set a minion's stats to 20/20.
    {
        return { 0, 20, 20, TavernSpellEffect::SET_TARGET_STATS };
    }
    if (id == "BG28_888") // Misplaced Tea Set: one minion of each type +4/+4.
    {
        return { 0, 4, 4, TavernSpellEffect::ONE_PER_RACE_STATS };
    }
    if (id == "BG28_897") // Tavern Dish Banana: a minion +2/+2.
    {
        return { 0, 2, 2, TavernSpellEffect::TARGET_STATS };
    }
    if (id == "BG32_815") // Shifting Tide: +1/+1 twice; Naga repeats once.
    {
        return { 0, 1, 1, TavernSpellEffect::TARGET_STATS_REPEAT,
                 Race::NAGA };
    }
    if (id == "BG35_149") // Deepwater Clan: target and Murlocs +2/+2.
    {
        return { 0, 2, 2, TavernSpellEffect::TARGET_AND_RACE,
                 Race::MURLOC };
    }
    if (id == "BG36_624") // Repair Job: a minion +4/+8.
    {
        return { 0, 4, 8, TavernSpellEffect::TARGET_STATS };
    }

    // Patch 36.4 simple keyword/economy batch.  These entries intentionally
    // stay within effects that Player can resolve without choices, generated
    // entities, or hidden/persistent card rules.
    if (id == "BG28_503") // Fortify: a minion +3 Health and Taunt.
    {
        return { 0, 0, 3, TavernSpellEffect::TARGET_STATS_AND_TAUNT };
    }
    if (id == "BG28_507") // Sacred Gift: give a minion Divine Shield.
    {
        return { 0, 0, 0, TavernSpellEffect::TARGET_DIVINE_SHIELD };
    }
    if (id == "BG28_520") // Tricky Trousers: +1/+2; toggle Taunt.
    {
        return { 0, 1, 2, TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT };
    }
    if (id == "BG28_825") // Defender's Rites: +7/+7 and Taunt.
    {
        return { 0, 7, 7, TavernSpellEffect::TARGET_STATS_AND_TAUNT };
    }
    if (id == "BG28_500") // Armor Stash: set hero Armor to 5.
    {
        return { 0, 0, 0, TavernSpellEffect::SET_PLAYER_ARMOR,
                 Race::INVALID, 0, 5 };
    }
    if (id == "BG28_800") // Careful Investment: gain 2 Gold next turn.
    {
        return { 0, 0, 0, TavernSpellEffect::NEXT_TURN_GOLD,
                 Race::INVALID, 0, 2 };
    }
    if (id == "BG28_805") // Strike Oil: increase maximum Gold by 1.
    {
        return { 0, 0, 0, TavernSpellEffect::INCREASE_MAX_GOLD,
                 Race::INVALID, 0, 1 };
    }
    if (id == "BG28_827") // Leaf Through the Pages: gain 2 free Refreshes.
    {
        return { 0, 0, 0, TavernSpellEffect::FREE_REFRESHES,
                 Race::INVALID, 0, 2 };
    }
    if (id == "BG28_886") // Staff of Enrichment: Tavern minions +2/+2.
    {
        return { 0, 2, 2, TavernSpellEffect::SHOP_STATS_PERSISTENT };
    }
    if (id == "BG28_571") // Hasty Excavation: pay this spell with Health.
    {
        return { 1, 0, 0, TavernSpellEffect::SPELL_COSTS_HEALTH };
    }

    return {};
}

//! Returns whether a supported Tavern spell needs a friendly board target.
inline bool TavernSpellRequiresTarget(TavernSpellEffect effect) noexcept
{
    return effect == TavernSpellEffect::TARGET_STATS ||
           effect == TavernSpellEffect::SET_TARGET_STATS ||
           effect == TavernSpellEffect::TARGET_AND_RACE ||
           effect == TavernSpellEffect::TARGET_STATS_REPEAT ||
           effect == TavernSpellEffect::TARGET_STATS_AND_TAUNT ||
           effect == TavernSpellEffect::TARGET_DIVINE_SHIELD ||
           effect == TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TAVERN_SPELL_BEHAVIORS_HPP
