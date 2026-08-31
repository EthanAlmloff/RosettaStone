// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - registry and target-free costs")
{
    constexpr std::array<std::int32_t, 3> ids = {105395, 103501, 103503};
    constexpr std::array<std::int32_t, 3> costs = {0, 0, 0};
    REQUIRE(SEASON14_HERO_POWER_BEHAVIORS_BATCH5.size() == ids.size());
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        const auto* entry = FindSeason14HeroPowerBehaviorBatch5(ids[i]);
        REQUIRE(entry != nullptr);
        CHECK(entry->cost == costs[i]);
        CHECK(!entry->passive);
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - unregistered powers stay closed")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    CHECK(!ResolveSeason14HeroPowerBatch5Activation(116921, state, result));
    CHECK(!ResolveSeason14HeroPowerBatch5Activation(999999, state, result));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - targeted tier attack is twice per turn")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    CHECK(ResolveSeason14HeroPowerBatch5Activation(103501, state, result, 1, 4));
    CHECK(result.attack == 4);
    CHECK(ResolveSeason14HeroPowerBatch5Activation(103501, state, result, 1, 4));
    CHECK(!ResolveSeason14HeroPowerBatch5Activation(103501, state, result, 1, 4));
    ResolveSeason14HeroPowerBatch5Event(103501, Season14HeroPowerBatch5Event::BEGIN_TURN,
                                        state, result);
    CHECK(ResolveSeason14HeroPowerBatch5Activation(103503, state, result, 1, 4));
    CHECK(result.health == 4);
    CHECK(result.attack == 0);
}
