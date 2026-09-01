// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - registry is exact")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH3.size() == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG20_HERO_100p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(80229) != nullptr);
    // Conviction is a typed post-combat choice, not a Tavern-tier proxy.
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG21_HERO_000p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(73941) != nullptr);
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
