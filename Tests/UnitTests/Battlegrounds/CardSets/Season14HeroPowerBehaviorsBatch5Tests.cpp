// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>

#include <array>
#include <string_view>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - registry is exact")
{
    constexpr std::array<std::string_view, 8> ids = {
        "TB_BaconShop_HP_062", "TB_BaconShop_HP_065",
        "TB_BaconShop_HP_042", "TB_BaconShop_HP_048",
        "TB_BaconShop_HP_087t", "TB_BaconShop_HP_107",
        "BG22_HERO_305p", "TB_BaconShop_HP_087"};
    constexpr std::array<std::int32_t, 8> dbfIDs = {
        61408, 61915, 59860, 60218, 64426, 67554, 82114, 64424};
    REQUIRE(SEASON14_HERO_POWER_BEHAVIORS_BATCH5.size() == ids.size());
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH5[i].id == ids[i]);
        CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH5[i].dbfID == dbfIDs[i]);
        REQUIRE(FindSeason14HeroPowerBehaviorBatch5(ids[i]) != nullptr);
        CHECK(FindSeason14HeroPowerBehaviorBatch5(ids[i])->dbfID == dbfIDs[i]);
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - deterministic lifecycle")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    ResolveSeason14HeroPowerBatch5Event(
        61408, Season14HeroPowerBatch5Event::REFRESH_TAVERN, state, result);
    CHECK(result.extraDragonOffers == 1);
    for (int i = 0; i < 4; ++i)
    {
        ResolveSeason14HeroPowerBatch5Event(
            61915, Season14HeroPowerBatch5Event::REFRESH_TAVERN, state,
            result);
    }
    CHECK(result.tavernSlotsDelta == 1);
    for (int i = 0; i < 4; ++i)
    {
        ResolveSeason14HeroPowerBatch5Event(
            82114, Season14HeroPowerBatch5Event::FRIENDLY_MINION_DIED, state,
            result);
    }
    CHECK(result.summonAttack == 3);
    CHECK(result.summonHealth == 1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch5] - counters have exact thresholds")
{
    Season14HeroPowerBatch5State state{};
    Season14HeroPowerBatch5Result result{};
    for (int i = 1; i <= 25; ++i)
    {
        ResolveSeason14HeroPowerBatch5Event(
            64424, Season14HeroPowerBatch5Event::ENEMY_MINION_KILLED, state,
            result);
        CHECK(result.trigger == (i == 25));
    }
}
