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
    TARGET_CHOOSE_ONE_STATS,
    ALL_MINION_CHOOSE_ONE_STATS,
    TARGET_OR_ALL_CHOOSE_ONE_STATS,
    SET_TARGET_STATS,
    TARGET_AND_RACE,
    TARGET_STATS_REPEAT,
    TARGET_STATS_AND_TAUNT,
    TARGET_STATS_AND_WINDFURY,
    TARGET_DIVINE_SHIELD_TEMP,
    TARGET_STATS_AND_REBORN,
    TARGET_DIVINE_SHIELD,
    TARGET_STATS_TOGGLE_TAUNT,
    SET_PLAYER_ARMOR,
    NEXT_TURN_GOLD,
    NEXT_COMBAT_REWARD,
    TARGET_NEXT_COMBAT_BUFF,
    COMBAT_START_LEFTMOST_ATTACK_DOUBLE,
    COMBAT_START_LEFTMOST_NEAREST_STATS,
    INCREASE_MAX_GOLD,
    FREE_REFRESHES,
    SHOP_STATS_PERSISTENT,
    SPELL_COSTS_HEALTH,
    TARGET_SHARED_RACE_STATS,
    TARGET_RACE_SHOP_STATS_PERSISTENT,
    TARGET_GOLDEN,
    RANDOM_SHOP_GOLDEN,
    RANDOM_MINION_TO_HAND,
    RANDOM_COMMON_RACE_MINION_TO_HAND,
    STEAL_RANDOM_SHOP_MINION,
    RANDOM_SHOP_STATS_ON_REFRESH,
    SELL_TARGET_GIVE_RANDOM_STATS,
    TARGET_CONSUME_SHOP_STATS,
    SELL_TARGET_GIVE_LEFTMOST_RACE_STATS,
    BLOOD_GEM,
    DISCOVER_MINION,
    TRANSFORM_HIGHER_TIER,
    DISCOVER_DIFFERENT_RACE,
    RANDOM_MINION_AND_COPY,
    FIXED_CARDS,
    BLOOD_GEM_TRANSFER,
    RANDOM_SPELLCRAFT,
    DISCOVER_HERO_POWER,
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
    bool copyKeywords = false;
    std::string_view cardA{};
    std::string_view cardB{};
    bool lockHand = false;
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
    if (id == "BG31_880") // Alliance Flag: choose +3/+1 or +1/+3.
        return { 0, 0, 0, TavernSpellEffect::TARGET_CHOOSE_ONE_STATS };
    if (id == "BG31_881") // Time Management: immediate or next-turn minion stats.
        return { 0, 0, 0, TavernSpellEffect::ALL_MINION_CHOOSE_ONE_STATS };
    if (id == "BG31_886") // Forest's Bounty: target twice or all minions.
        return { 0, 0, 0, TavernSpellEffect::TARGET_OR_ALL_CHOOSE_ONE_STATS };
    if (id == "BG23_000t") return { 0, 2, 0, TavernSpellEffect::TARGET_STATS };
    if (id == "BG23_000_Gt") return { 0, 4, 0, TavernSpellEffect::TARGET_STATS };
    if (id == "BG23_004t") return { 0, 2, 6, TavernSpellEffect::TARGET_STATS_AND_TAUNT };
    if (id == "BG23_004_Gt") return { 0, 4, 12, TavernSpellEffect::TARGET_STATS_AND_TAUNT };
    if (id == "BG23_007t") return { 0, 2, 2, TavernSpellEffect::TARGET_STATS_AND_WINDFURY, Race::NAGA };
    if (id == "BG23_007_Gt") return { 0, 4, 4, TavernSpellEffect::TARGET_STATS_AND_WINDFURY, Race::NAGA };
    if (id == "BG23_008t") return { 0, 0, 0, TavernSpellEffect::TARGET_DIVINE_SHIELD_TEMP };
    if (id == "BG23_008_Gt") return { 0, 0, 0, TavernSpellEffect::TARGET_DIVINE_SHIELD_TEMP };
    if (id == "BG31_830t") return { 0, 2, 2, TavernSpellEffect::TARGET_STATS_AND_REBORN, Race::NAGA };
    if (id == "BG31_830_Gt") return { 0, 4, 4, TavernSpellEffect::TARGET_STATS_AND_REBORN, Race::NAGA };
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
    if (id == "BG28_884") // Gain 3 Gold on a win, or 1 on a tie, next combat.
    {
        return { 0, 0, 0, TavernSpellEffect::NEXT_COMBAT_REWARD };
    }
    if (id == "BG36_883") // +2/+3 now; +4/+6 to that minion after a win.
    {
        return { 0, 2, 3, TavernSpellEffect::TARGET_NEXT_COMBAT_BUFF };
    }
    if (id == "BG34_889") // Double the left-most minion's Attack in combat.
    {
        return { 0, 0, 0,
                 TavernSpellEffect::COMBAT_START_LEFTMOST_ATTACK_DOUBLE };
    }
    if (id == "BG31_889") // Copy stats from the nearest enemy at combat start.
    {
        return { 0, 0, 0,
                 TavernSpellEffect::COMBAT_START_LEFTMOST_NEAREST_STATS };
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

    if (id == "BG20_GEM") // Blood Gem: apply the player's current gem scale.
    {
        return { 0, 1, 1, TavernSpellEffect::BLOOD_GEM };
    }

    // Patch 36.4 generated/tribal batch.  These entries are deliberately
    // limited to effects that can be resolved from public player state and
    // the existing structured TavernSpell target argument.
    if (id == "BG28_845") // Natural Blessing: share a type with a minion.
    {
        return { 0, 3, 3, TavernSpellEffect::TARGET_SHARED_RACE_STATS };
    }
    if (id == "BG35_912") // Eonar's Favor: same type in Tavern this game.
    {
        return { 0, 3, 3,
                 TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT };
    }
    if (id == "EBG_Spell_017") // Eyes of the Earth Mother: make <= T4 Golden.
    {
        return { 0, 0, 0, TavernSpellEffect::TARGET_GOLDEN };
    }
    if (id == "BG28_830") // Golden Touch: random Tavern minion Golden.
    {
        return { 0, 0, 0, TavernSpellEffect::RANDOM_SHOP_GOLDEN };
    }
    if (id == "BG28_504") // Recruit a Trainee: random Tier 1 minion.
    {
        return { 0, 0, 0, TavernSpellEffect::RANDOM_MINION_TO_HAND };
    }
    if (id == "BG33_814") // Friendly Bounty: random most-common-type minion.
    {
        return { 0, 0, 0,
                 TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND };
    }
    if (id == "BG33_101") // A New Sprout: Discover a Tier 1 minion.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_MINION,
                 Race::INVALID, 3, 1 };
    if (id == "BG31_896") // Hallowed Ritual: Discover a Tier 7 minion.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_MINION,
                 Race::INVALID, 3, 7 };
    if (id == "BG34_330") // Search Through Time: current-tier minion, locked one turn.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_MINION,
                 Race::INVALID, 3, 0, false, {}, {}, true };
    if (id == "BG28_882") // Contracted Corpse: Discover a Deathrattle minion.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_MINION,
                 Race::INVALID, 3, 8 };
    if (id == "EBG_Spell_037") // Unmasked Identity: Discover a new Hero Power.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_HERO_POWER };
    if (id == "BG30_804") // Robust Evolution: random higher-Tier transform.
        return { 0, 0, 0, TavernSpellEffect::TRANSFORM_HIGHER_TIER };
    if (id == "BG28_518") // Chef's Choice: a different minion of its type.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_DIFFERENT_RACE };
    if (id == "BG28_601") // Cloning Conch: a random Murloc and a copy.
        return { 0, 0, 0, TavernSpellEffect::RANDOM_MINION_AND_COPY,
                 Race::MURLOC, 2 };
    if (id == "BG31_819") // Temperature Shift: Fire Baller and Snow Baller.
        return { 0, 0, 0, TavernSpellEffect::FIXED_CARDS,
                 Race::INVALID, 0, 0, false, "BG31_816", "BG31_818" };
    if (id == "BG36_884") // Weapons Forge: three Pointy Arrows.
        return { 0, 0, 0, TavernSpellEffect::FIXED_CARDS,
                 Race::INVALID, 3, 0, false, "EBG_Spell_014" };
    if (id == "BG28_698") // Gem Confiscation: apply two gems and steal neighbors.
        return { 0, 0, 0, TavernSpellEffect::BLOOD_GEM_TRANSFER };
    if (id == "BG28_606") // Spitescale Special: three random Spellcraft spells.
        return { 0, 0, 0, TavernSpellEffect::RANDOM_SPELLCRAFT };
    if (id == "BG28_521") // Planar Telescope: Discover a common-type minion.
        return { 0, 0, 0, TavernSpellEffect::DISCOVER_MINION,
                 Race::INVALID, 3, 0 };
    if (id == "BG28_512") // Enchanted Lasso: steal a random Tavern minion.
    {
        return { 0, 0, 0, TavernSpellEffect::STEAL_RANDOM_SHOP_MINION };
    }
    if (id == "BG34_444") // Easterly Winds: future refreshes buff one minion.
    {
        return { 0, 8, 8, TavernSpellEffect::RANDOM_SHOP_STATS_ON_REFRESH };
    }
    if (id == "EBG_Spell_032") // Channel the Devourer: sell and transfer.
    {
        return { 0, 0, 0, TavernSpellEffect::SELL_TARGET_GIVE_RANDOM_STATS };
    }
    if (id == "BG28_607") // Corrupted Cupcakes: Demon consumes three Tavern minions.
    {
        return { 0, 0, 0, TavernSpellEffect::TARGET_CONSUME_SHOP_STATS,
                 Race::DEMON, 3 };
    }
    if (id == "BG36_880") // Methodical Madness: Demon consumes two with keywords.
        return { 0, 0, 0, TavernSpellEffect::TARGET_CONSUME_SHOP_STATS,
                 Race::DEMON, 2, 0, true };
    if (id == "BG33_899") // Mounting Avalanche: sell into leftmost Elemental.
    {
        return { 0, 0, 0,
                 TavernSpellEffect::SELL_TARGET_GIVE_LEFTMOST_RACE_STATS,
                 Race::ELEMENTAL };
    }

    return {};
}

//! Returns whether a supported Tavern spell needs a friendly board target.
inline bool TavernSpellRequiresTarget(TavernSpellEffect effect) noexcept
{
    return effect == TavernSpellEffect::TARGET_STATS ||
           effect == TavernSpellEffect::TARGET_NEXT_COMBAT_BUFF ||
           effect == TavernSpellEffect::TARGET_CHOOSE_ONE_STATS ||
           effect == TavernSpellEffect::TARGET_OR_ALL_CHOOSE_ONE_STATS ||
           effect == TavernSpellEffect::BLOOD_GEM ||
           effect == TavernSpellEffect::SET_TARGET_STATS ||
           effect == TavernSpellEffect::TARGET_AND_RACE ||
           effect == TavernSpellEffect::TARGET_STATS_REPEAT ||
           effect == TavernSpellEffect::TARGET_STATS_AND_TAUNT ||
           effect == TavernSpellEffect::TARGET_STATS_AND_WINDFURY ||
           effect == TavernSpellEffect::TARGET_DIVINE_SHIELD_TEMP ||
           effect == TavernSpellEffect::TARGET_STATS_AND_REBORN ||
           effect == TavernSpellEffect::TARGET_DIVINE_SHIELD ||
           effect == TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT ||
           effect == TavernSpellEffect::TARGET_SHARED_RACE_STATS ||
           effect == TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT ||
           effect == TavernSpellEffect::TARGET_GOLDEN ||
           effect == TavernSpellEffect::SELL_TARGET_GIVE_RANDOM_STATS ||
           effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS ||
           effect == TavernSpellEffect::SELL_TARGET_GIVE_LEFTMOST_RACE_STATS ||
           effect == TavernSpellEffect::TRANSFORM_HIGHER_TIER ||
           effect == TavernSpellEffect::DISCOVER_DIFFERENT_RACE ||
           effect == TavernSpellEffect::BLOOD_GEM_TRANSFER;
}

//! Returns whether a target satisfies additional spell-specific constraints.
//! Generic friendly-minion legality is checked by Player; this helper keeps
//! the low-level Tier 4 cap for Eyes of the Earth Mother declarative and
//! replayable from the same spell table.
inline bool TavernSpellTargetIsLegal(TavernSpellEffect effect, int tier,
                                     bool golden) noexcept
{
    if (effect == TavernSpellEffect::TARGET_GOLDEN)
    {
        return !golden && tier >= 1 && tier <= 4;
    }
    return true;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TAVERN_SPELL_BEHAVIORS_HPP
