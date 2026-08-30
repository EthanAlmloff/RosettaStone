// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch4.hpp>

#include <array>
#include <tuple>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch4] - registry is exact")
{
    constexpr std::array<std::string_view, 8> expectedIds = {
        "BG20_HERO_102p", "TB_BaconShop_HP_015", "TB_BaconShop_HP_061",
        "TB_BaconShop_HP_066", "TB_BaconShop_HP_065t2",
        "TB_BaconShop_HP_049", "BG22_HERO_002p", "BG22_HERO_003p"};
    constexpr std::array<std::int32_t, 8> expected = {
        71455, 57949, 61406, 61917, 62035, 60285, 80244, 80248};
    REQUIRE(SEASON14_HERO_POWER_BEHAVIORS_BATCH4.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const auto& entry = SEASON14_HERO_POWER_BEHAVIORS_BATCH4[i];
        CHECK(entry.id == expectedIds[i]);
        CHECK(entry.dbfID == expected[i]);
        REQUIRE(FindSeason14HeroPowerBehaviorBatch4(entry.id) != nullptr);
        REQUIRE(FindSeason14HeroPowerBehaviorBatch4(entry.dbfID) != nullptr);
        CHECK(FindSeason14HeroPowerBehaviorBatch4(entry.id)->dbfID ==
              entry.dbfID);
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch4] - passive modifiers are table driven")
{
    const auto tinker = Season14HeroPowerBatch4Modifiers(57949);
    CHECK(tinker.mechShopAttack == 1);
    CHECK(tinker.mechShopHealth == 1);

    const auto allWillBurn = Season14HeroPowerBatch4Modifiers(61406);
    CHECK(allWillBurn.globalMinionAttack == 3);

    const auto spectralSight = Season14HeroPowerBatch4Modifiers(62035);
    CHECK(spectralSight.tavernSlotsDelta == 1);

    CHECK(Season14HeroPowerBatch4Modifiers(71455).globalMinionAttack == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch4] - lifecycle events scale and trigger")
{
    Season14HeroPowerBatch4State state{};
    Season14HeroPowerBatch4Result result{};

    ResolveSeason14HeroPowerBatch4Event(
        71455, Season14HeroPowerBatch4Event::BEGIN_TURN, state, result);
    CHECK(state.nextBuyAttack == 0);
    CHECK(ResolveSeason14HeroPowerBatch4Activation(71455, state, result));
    CHECK(state.nextBuyAttack == 2);
    ResolveSeason14HeroPowerBatch4Event(
        71455, Season14HeroPowerBatch4Event::BUY_MINION, state, result);
    CHECK(result.purchaseAttack == 2);
    CHECK(state.nextBuyAttack == 0);
    ResolveSeason14HeroPowerBatch4Event(
        71455, Season14HeroPowerBatch4Event::BEGIN_TURN, state, result);
    CHECK(state.nextBuyAttack == 3);

    for (int i = 0; i < 3; ++i)
    {
        ResolveSeason14HeroPowerBatch4Event(
            61917, Season14HeroPowerBatch4Event::PLAY_MINION, state, result);
    }
    CHECK(result.attack == 2);
    CHECK(result.health == 2);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch4] - Avenge thresholds are exact")
{
    for (const auto [dbfID, threshold, attack, health] :
         {std::tuple{80244, 3, 1, 0}, std::tuple{80248, 2, 0, 1}})
    {
        Season14HeroPowerBatch4State state{};
        Season14HeroPowerBatch4Result result{};
        for (int i = 1; i <= threshold; ++i)
        {
            ResolveSeason14HeroPowerBatch4Event(
                dbfID, Season14HeroPowerBatch4Event::FRIENDLY_MINION_DIED,
                state, result);
            CHECK(result.avengeTriggered == (i == threshold));
        }
        CHECK(result.attack == attack);
        CHECK(result.health == health);

        ResolveSeason14HeroPowerBatch4Event(
            dbfID, Season14HeroPowerBatch4Event::COMBAT_START, state, result);
        CHECK(state.combatDeaths == 0);
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch4] - Graveyard Shift is atomic data")
{
    Season14HeroPowerBatch4State state{};
    Season14HeroPowerBatch4Result result{};
    CHECK(ResolveSeason14HeroPowerBatch4Activation(60285, state, result));
    CHECK(result.goldDelta == 2);
    CHECK(result.healthDelta == -4);
    CHECK(!ResolveSeason14HeroPowerBatch4Activation(60286, state, result));
}
