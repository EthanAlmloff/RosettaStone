// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - registry is exact")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH3.size() == 21);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG20_HERO_100p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(80229) != nullptr);
    // Conviction is a typed post-combat choice, not a Tavern-tier proxy.
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG21_HERO_000p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(73941) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_087") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(64424) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_087t") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(64426) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(59808) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(59863) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_104") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(66246) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_052") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(60378) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_084") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(63607) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_107") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(67554) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG33_HERO_001p_ALT") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(129164) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG22_HERO_007p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(79619) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG22_HERO_201p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(81570) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG22_HERO_305p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(82114) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG22_HERO_000p_Alt") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(123150) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_105") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(66484) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_085t") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(122960) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_069") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(61851) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("TB_BaconShop_HP_103") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(66197) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG27_HERO_801p2") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(104628) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG24_HERO_204p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(96872) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(0) == nullptr);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - unsupported choice family fails closed")
{
    Season14HeroPowerBatch3Activation result{};
    CHECK(!ResolveSeason14HeroPowerBatch3Activation(73941, 1, result));
    CHECK(result.attack == 0);
    CHECK(result.health == 0);
    CHECK(result.randomCount == 0);
    CHECK(!ResolveSeason14HeroPowerBatch3Activation(80229, 5, result));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Glory awards one attack")
{
    CHECK(Season14HeroPowerBatch3CombatKillAttack(80229) == 1);
    CHECK(Season14HeroPowerBatch3CombatKillAttack(73941) == 0);
    CHECK(Season14HeroPowerBatch3CombatKillAttack(0) == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Bloodfury buffs Demons")
{
    Season14HeroPowerBatch3Activation result{};
    CHECK(ResolveSeason14HeroPowerBatch3Activation(59808, 1, result));
    CHECK(result.attack == 1);
    CHECK(result.health == 1);
    CHECK(result.randomCount == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Wax Warband targets each race")
{
    Season14HeroPowerBatch3Activation result{};
    CHECK(ResolveSeason14HeroPowerBatch3Activation(59863, 1, result));
    CHECK(result.attack == 1);
    CHECK(result.health == 1);
    CHECK(result.randomCount == -1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Saturday C'Thuns arms one buff")
{
    Season14HeroPowerBatch3Activation result{};
    CHECK(ResolveSeason14HeroPowerBatch3Activation(66246, 1, result));
    CHECK(result.attack == 1);
    CHECK(result.health == 1);
    CHECK(result.randomCount == 1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Ragnaros unlock and Sulfuras end turn")
{
    const auto effect = Season14HeroPowerBatch3CombatKillThresholdFor(64424);
    CHECK(effect.threshold == 25);
    CHECK(effect.attack == 0);
    CHECK(effect.health == 0);
    const auto sulfuras = Season14HeroPowerBatch3SulfurasEndTurnBuff(64426);
    CHECK(sulfuras.attack == 3);
    CHECK(sulfuras.health == 3);
    CHECK(Season14HeroPowerBatch3CombatKillThresholdFor(64426).threshold == 0);
    Season14State state;
    state.SetHeroPower(64424, 0, true);
    for (int i = 0; i < 24; ++i) CHECK(!state.RecordFriendlyCombatKill());
    CHECK(state.RecordFriendlyCombatKill());
    CHECK(state.heroPowerDbfID == 64426);
    CHECK(!state.RecordFriendlyCombatKill());
    state.Emit(Season14Event::COMBAT_START);
    CHECK(!state.RecordFriendlyCombatKill());
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - Conviction improvement is a replayable choice")
{
    Season14State state;
    state.SetHeroPower(73941, 0, true);
    state.QueueConvictionImprovements(2);
    CHECK(state.BeginConvictionImprovementChoice());
    REQUIRE(state.pendingDecision == Season14Decision::CHOICE);
    REQUIRE(state.pendingOfferings.size() == 3);
    CHECK(state.pendingOfferings[0].dbfID == CONVICTION_IMPROVE_ATTACK);
    CHECK(state.ApplyConvictionImprovement(1));
    CHECK(state.ConvictionHealthBonus() == 1);
    CHECK(state.pendingDecision == Season14Decision::CHOICE);
    CHECK(state.PendingConvictionImprovements() == 0);
    CHECK(state.ApplyConvictionImprovement(2));
    CHECK(state.ConvictionExtraTargets() == 1);
    CHECK(state.pendingDecision == Season14Decision::NONE);
}
