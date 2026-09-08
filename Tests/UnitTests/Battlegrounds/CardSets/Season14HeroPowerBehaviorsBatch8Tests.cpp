// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch8.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch8] - pinned registry metadata")
{
    constexpr std::array<std::int32_t, 8> ids = {
        57561, 57957, 57962, 58017, 58527, 59815, 59862, 60376};
    REQUIRE(SEASON14_HERO_POWER_BEHAVIORS_BATCH8.size() == ids.size());
    for (const auto id : ids) CHECK(FindSeason14HeroPowerBehaviorBatch8(id) != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(999999) == nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch8("TB_BaconShop_HP_043")->dbfID == 59862);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(57561)->cost == 0);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(57561)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(57957)->cost == 1);
    CHECK(!FindSeason14HeroPowerBehaviorBatch8(57957)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(57962)->cost == 1);
    CHECK(!FindSeason14HeroPowerBehaviorBatch8(57962)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(58017)->cost == 2);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(58017)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(58527)->cost == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(58527)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(59815)->cost == 1);
    CHECK(!FindSeason14HeroPowerBehaviorBatch8(59815)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(59862)->cost == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(59862)->passive);
    CHECK(FindSeason14HeroPowerBehaviorBatch8(60376)->cost == 2);
    CHECK(!FindSeason14HeroPowerBehaviorBatch8(60376)->passive);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch8] - lifecycle counters and deltas")
{
    Season14HeroPowerBatch8State state{};
    Season14HeroPowerBatch8Result result{};
    ResolveSeason14HeroPowerBatch8Event(
        59815, Season14HeroPowerBatch8Event::BEGIN_TURN, state, result);
    CHECK(result.bananaCards == 0);
    CHECK(state.turnNumber == 1);
    CHECK(state.bananaramaActivations == 0);
    CHECK(state.murlocKingActivations == 0);
    CHECK(state.pendingStartCombatPowerRepeats == 0);
    CHECK(Season14HeroPowerBatch8Modifiers(57561).upgradeCostDelta == -1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch8] - combat effects are exact and fail closed")
{
    Season14HeroPowerBatch8State state{};
    Season14HeroPowerBatch8Result result{};
    CHECK(!ResolveSeason14HeroPowerBatch8Activation(57559, state, result));
    CHECK(!ResolveSeason14HeroPowerBatch8Activation(999999, state, result));
    CHECK(ResolveSeason14HeroPowerBatch8Activation(57962, state, result));
    CHECK(result.attack == 10);
    CHECK(result.temporaryUntilNextTurn);
    CHECK(ResolveSeason14HeroPowerBatch8Activation(59815, state, result));
    CHECK(result.bananaCards == 2);
    CHECK(ResolveSeason14HeroPowerBatch8Activation(57957, state, result));
    CHECK(ResolveSeason14HeroPowerBatch8CombatStart(58017, result));
    CHECK(result.damage == 8);
    CHECK(result.damageTargets == 2);
    CHECK(ResolveSeason14HeroPowerBatch8CombatStart(59862, result));
    CHECK(result.damage == 1);
    CHECK(result.allEnemyMinions);
    CHECK(!ResolveSeason14HeroPowerBatch8CombatStart(999999, result));
}
