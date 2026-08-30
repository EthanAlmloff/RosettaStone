// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch3] - registry is exact")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH3.size() == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG20_HERO_100p") != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(80229) != nullptr);
    // Conviction's post-combat improvement choice is not represented by the
    // current bridge schema; a Tavern-tier proxy would be incorrect.
    CHECK(FindSeason14HeroPowerBehaviorBatch3("BG21_HERO_000p") == nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch3(73941) == nullptr);
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
