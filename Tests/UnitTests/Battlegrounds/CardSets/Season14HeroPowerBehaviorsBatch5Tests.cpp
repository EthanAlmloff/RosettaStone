// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - registry and target-free costs")
{
    constexpr std::array<std::int32_t, 16> ids = {105395, 103501, 103503, 80539, 61915, 61408, 104875, 76520, 77911, 86014, 79720, 59201, 59860, 64402, 60448, 60218};
    constexpr std::array<std::int32_t, 16> costs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    REQUIRE(SEASON14_HERO_POWER_BEHAVIORS_BATCH5.size() == ids.size());
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        const auto* entry = FindSeason14HeroPowerBehaviorBatch5(ids[i]);
        REQUIRE(entry != nullptr);
        CHECK(entry->cost == costs[i]);
        CHECK(entry->passive == (ids[i] != 105395 && ids[i] != 103501 && ids[i] != 103503 && ids[i] != 79720));
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Embrace the Elements pins four invocations")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5(79720);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::EMBRACE_ELEMENTS);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Tentacular scales its combat token after sales")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5(86014);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::TENTACULAR);
    CHECK(entry->passive);
    CHECK(Cards::FindCardByID("BG23_HERO_201pt").dbfID == 86227);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Fragrant Phylactery uses all other minions")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5(77911);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::FRAGRANT_PHYLACTERY);
    CHECK(entry->passive);
    CHECK(Cards::FindCardByID("BG20_HERO_282p").dbfID == 77911);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Pilot the Shredder starts the exact token")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5(76520);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::STARTING_SHREDDER);
    CHECK(Cards::FindCardByID("BG21_HERO_030t").dbfID == 123014);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Glaive Ricochet arms after three buys")
{
    Season14HeroPowerBatch5State state{};
    RecordSeason14HeroPowerBatch5GlaivePurchase(101, state);
    RecordSeason14HeroPowerBatch5GlaivePurchase(102, state);
    CHECK(!Season14HeroPowerBatch5GlaiveReady(state));
    RecordSeason14HeroPowerBatch5GlaivePurchase(103, state);
    CHECK(Season14HeroPowerBatch5GlaiveReady(state));
    CHECK(ConsumeSeason14HeroPowerBatch5Glaive(state));
    CHECK(state.glaiveUsesRemaining == 2);
    CHECK(!Season14HeroPowerBatch5GlaiveReady(state));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Dream Portal is extra Dragon refresh")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5(61408);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::EXTRA_DRAGON_REFRESH);
    CHECK(entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Demon Hunter Training unlocks after fourteen attacks")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    for (int i = 0; i < 13; ++i)
        ResolveSeason14HeroPowerBatch5Event(
            61915, Season14HeroPowerBatch5Event::FRIENDLY_MINION_ATTACKED,
            state, result);
    CHECK(!Season14HeroPowerBatch5FirstBuyFree(state));
    ResolveSeason14HeroPowerBatch5Event(
        61915, Season14HeroPowerBatch5Event::FRIENDLY_MINION_ATTACKED,
        state, result);
    CHECK(Season14HeroPowerBatch5FirstBuyFree(state));
    CHECK(ConsumeSeason14HeroPowerBatch5FirstBuyFree(61915, state));
    CHECK(!Season14HeroPowerBatch5FirstBuyFree(state));
    ResolveSeason14HeroPowerBatch5Event(
        61915, Season14HeroPowerBatch5Event::BEGIN_TURN, state, result);
    CHECK(Season14HeroPowerBatch5FirstBuyFree(state));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - Twice as Nice is refresh passive")
{
    const auto* entry = FindSeason14HeroPowerBehaviorBatch5("BG22_HERO_004p");
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerBatch5Kind::COPY_HIGHEST_REFRESH);
    CHECK(entry->passive);
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
