// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Explicit, fully executable families from the third Season 14 batch.
//!
//! This registry is intentionally small.  An ID is added here only when the
//! simulator has a complete transition for it; metadata-only powers remain
//! absent and therefore fail closed in the bridge.
enum class Season14HeroPowerBatch3Kind : std::uint8_t
{
    COMBAT_KILL_ATTACK,
    COMBAT_KILL_UNLOCK,
    END_TURN_EDGE_BUFF,
    DEMON_BUFF,
    MINION_TYPE_BUFF,
    END_TURN_RANDOM_BUFF,
    TAVERN_REPLACE_SAME_TIER,
    TAVERN_SWAP,
    START_GAME_FISH,
    TAVERN_SPELL_AURA,
    START_COMBAT_EDGE_ATTACK,
    START_COMBAT_TIER_MINION,
    DEFERRED_TIER7_REWARD,
    REFRESH_BONUS_KEYWORD,
    COMBAT_SUMMON_BUFF,
    COMBAT_ATTACK_THRESHOLD,
    AZSHARA_AMBITION,
    EXPEDITION_PLANS,
    BROODMOTHER,
    LOCK_AND_LOAD,
    CONVICTION_IMPROVEMENT,
};

struct Season14HeroPowerBatch3Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch3Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch3Definition, 21>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH3 = {{
        {"BG20_HERO_100p", 80229,
         Season14HeroPowerBatch3Kind::COMBAT_KILL_ATTACK, 0, true},
        {"TB_BaconShop_HP_087", 64424,
         Season14HeroPowerBatch3Kind::COMBAT_KILL_UNLOCK, 0, true},
        {"TB_BaconShop_HP_087t", 64426,
         Season14HeroPowerBatch3Kind::END_TURN_EDGE_BUFF, 0, true},
        {"BG21_HERO_000p", 73941,
         Season14HeroPowerBatch3Kind::CONVICTION_IMPROVEMENT, 0, false},
        {"TB_BaconShop_HP_036", 59808,
         Season14HeroPowerBatch3Kind::DEMON_BUFF, 0, false},
        {"TB_BaconShop_HP_037a", 59863,
         Season14HeroPowerBatch3Kind::MINION_TYPE_BUFF, 1, false},
        {"TB_BaconShop_HP_104", 66246,
         Season14HeroPowerBatch3Kind::END_TURN_RANDOM_BUFF, 1, false},
        {"TB_BaconShop_HP_052", 60378,
         Season14HeroPowerBatch3Kind::TAVERN_REPLACE_SAME_TIER, 0, false},
        {"TB_BaconShop_HP_084", 63607,
         Season14HeroPowerBatch3Kind::TAVERN_SWAP, 0, false},
        {"TB_BaconShop_HP_105", 66484,
         Season14HeroPowerBatch3Kind::START_GAME_FISH, 0, true},
        {"TB_BaconShop_HP_085t", 122960,
         Season14HeroPowerBatch3Kind::TAVERN_SPELL_AURA, 0, true},
        {"TB_BaconShop_HP_069", 61851,
         Season14HeroPowerBatch3Kind::START_COMBAT_EDGE_ATTACK, 0, true},
        {"TB_BaconShop_HP_103", 66197,
         Season14HeroPowerBatch3Kind::START_COMBAT_TIER_MINION, 2, false},
        {"BG27_HERO_801p2", 104628,
         Season14HeroPowerBatch3Kind::DEFERRED_TIER7_REWARD, 0, true},
        {"BG24_HERO_204p", 96872,
         Season14HeroPowerBatch3Kind::REFRESH_BONUS_KEYWORD, 0, true},
        {"TB_BaconShop_HP_107", 67554,
         Season14HeroPowerBatch3Kind::COMBAT_SUMMON_BUFF, 0, true},
        {"BG33_HERO_001p_ALT", 129164,
         Season14HeroPowerBatch3Kind::COMBAT_ATTACK_THRESHOLD, 0, true},
        {"BG22_HERO_007p", 79619,
         Season14HeroPowerBatch3Kind::AZSHARA_AMBITION, 0, true},
        {"BG22_HERO_201p", 81570,
         Season14HeroPowerBatch3Kind::EXPEDITION_PLANS, 0, true},
        {"BG22_HERO_305p", 82114,
         Season14HeroPowerBatch3Kind::BROODMOTHER, 0, true},
        {"BG22_HERO_000p_Alt", 123150,
         Season14HeroPowerBatch3Kind::LOCK_AND_LOAD, 0, false},
    }};

constexpr const Season14HeroPowerBatch3Definition*
FindSeason14HeroPowerBehaviorBatch3(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH3)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerBatch3Definition*
FindSeason14HeroPowerBehaviorBatch3(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH3)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

//! Result of a target-free Batch-3 activation.
//!
//! Choice-based powers do not use this target-free result. It remains the
//! narrow extension point for stateless Batch-3 activations.
struct Season14HeroPowerBatch3Activation
{
    std::int32_t attack = 0;
    std::int32_t health = 0;
    //! Number of random minions to choose.  Zero means all friendly minions.
    //! -1 is the typed one-recipient-per-minion-type operation.
    std::int32_t randomCount = 0;
};

//! Resolve a complete target-free Batch-3 activation.
//!
//! Choice-based Conviction is resolved through Player's replayable modal;
//! other unregistered families return false.
constexpr bool ResolveSeason14HeroPowerBatch3Activation(
    std::int32_t dbfID, std::int32_t currentTier,
    Season14HeroPowerBatch3Activation& result) noexcept
{
    result = {};
    (void)currentTier;
    if (dbfID == 59808) {
        result.attack = 1;
        result.health = 1;
        return true;
    }
    if (dbfID == 59863) {
        result.attack = 1;
        result.health = 1;
        result.randomCount = -1; // one recipient per minion type
        return true;
    }
    if (dbfID == 66246) {
        result.attack = 1;
        result.health = 1;
        result.randomCount = 1;
        return true;
    }
    if (dbfID == 60378) {
        result.randomCount = 1;
        return true;
    }
    if (dbfID == 63607) {
        result.randomCount = 1;
        return true;
    }
    if (dbfID == 66197) {
        result.randomCount = 1;
        return true;
    }
    return false;
}

//! Return the permanent attack bonus awarded by Glory of Combat after a
//! friendly minion kills an enemy.  Unsupported powers return zero.
constexpr std::int32_t Season14HeroPowerBatch3CombatKillAttack(
    std::int32_t dbfID) noexcept
{
    return dbfID == 80229 ? 1 : 0;
}

struct Season14HeroPowerBatch3CombatKillThreshold
{
    std::int32_t threshold = 0;
    std::int32_t attack = 0;
    std::int32_t health = 0;
};

constexpr Season14HeroPowerBatch3CombatKillThreshold
Season14HeroPowerBatch3CombatKillThresholdFor(std::int32_t dbfID) noexcept
{
    return dbfID == 64424
               ? Season14HeroPowerBatch3CombatKillThreshold{25, 0, 0}
               : Season14HeroPowerBatch3CombatKillThreshold{};
}

constexpr Season14HeroPowerBatch3CombatKillThreshold
Season14HeroPowerBatch3SulfurasEndTurnBuff(std::int32_t dbfID) noexcept
{
    return dbfID == 64426
               ? Season14HeroPowerBatch3CombatKillThreshold{1, 3, 3}
               : Season14HeroPowerBatch3CombatKillThreshold{};
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH3_HPP
