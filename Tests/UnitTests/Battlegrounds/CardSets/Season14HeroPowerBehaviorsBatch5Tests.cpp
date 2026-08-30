// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>

#include <array>
#include <string_view>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - unverified registry fails closed")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH5.empty());
    CHECK(FindSeason14HeroPowerBehaviorBatch5("TB_BaconShop_HP_062") == nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch5(61408) == nullptr);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - unregistered lifecycle is inert")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    ResolveSeason14HeroPowerBatch5Event(
        61408, Season14HeroPowerBatch5Event::REFRESH_TAVERN, state, result);
    CHECK(result.trigger == false);
    CHECK(result.extraDragonOffers == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - counters never expose guessed thresholds")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    for (int i = 1; i <= 25; ++i)
    {
        ResolveSeason14HeroPowerBatch5Event(
            64424, Season14HeroPowerBatch5Event::ENEMY_MINION_KILLED, state,
            result);
        CHECK(result.trigger == false);
    }
}
