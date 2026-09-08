#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch9.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch9] - registry has exact pinned rows")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH9.size() == 8);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("BG24_HERO_100p")->dbfID == 92961);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("BG25_HERO_100p")->dbfID == 97814);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_020")->dbfID == 58022);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_057")->dbfID == 60450);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_077")->dbfID == 63162);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_080")->dbfID == 63320);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_081")->dbfID == 63600);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_106")->dbfID == 67357);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("BG24_HERO_100p")->cost == 0);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("BG25_HERO_100p")->cost == 3);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_020")->cost == 0);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_077")->cost == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch9("TB_BaconShop_HP_081")->cost == 1);
    CHECK(FindSeason14HeroPowerBehaviorBatch9(0) == nullptr);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch9] - start choices and Prize Wall are scheduled")
{
    Season14HeroPowerBatch9State state{};
    Season14HeroPowerBatch9Result result{};
    ResolveSeason14HeroPowerBatch9Event(
        60450, Season14HeroPowerBatch9Event::BEGIN_GAME, state, result);
    CHECK(result.beginChoice);
    CHECK(state.startGameChoiceReady);

    state = {};
    for (int turn = 0; turn < 3; ++turn) {
        ResolveSeason14HeroPowerBatch9Event(
            67357, Season14HeroPowerBatch9Event::BEGIN_TURN, state, result);
        CHECK(!result.prizeReady);
    }
    ResolveSeason14HeroPowerBatch9Event(
        67357, Season14HeroPowerBatch9Event::BEGIN_TURN, state, result);
    CHECK(result.prizeReady);
    CHECK(state.prizeWallTurns == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch9] - choice reservation rolls back and commits")
{
    Season14HeroPowerBatch9State state{};
    CHECK(ConsumeSeason14HeroPowerBatch9Choice(state));
    CHECK(!ConsumeSeason14HeroPowerBatch9Choice(state));
    RestoreSeason14HeroPowerBatch9Choice(state);
    CHECK(ConsumeSeason14HeroPowerBatch9Choice(state));
    CHECK(CommitSeason14HeroPowerBatch9Choice(state));
    CHECK(!CommitSeason14HeroPowerBatch9Choice(state));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch9] - active counters are replay-safe")
{
    Season14HeroPowerBatch9State state{};
    CHECK(state.creationsRemaining == 3);
    CHECK(ConsumeSeason14UndeadCreation(state));
    CHECK(state.creationsRemaining == 2);
    CHECK(ConsumeSeason14UndeadCreation(state));
    CHECK(ConsumeSeason14UndeadCreation(state));
    CHECK(!ConsumeSeason14UndeadCreation(state));

    state.prizeChoiceReady = true;
    CHECK(ConsumeSeason14PrizeWall(state));
    CHECK(!ConsumeSeason14PrizeWall(state));

    CHECK(ArmSeason14FriendlyWager(state));
    CHECK(!ArmSeason14FriendlyWager(state));
    CHECK(ResolveSeason14FriendlyWager(state, true));
    CHECK(!ResolveSeason14FriendlyWager(state, false));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch9] - Prize Wall cadence is four turns")
{
    Season14HeroPowerBatch9State state{};
    Season14HeroPowerBatch9Result result{};
    for (int turn = 0; turn < 3; ++turn) {
        ResolveSeason14HeroPowerBatch9Event(
            67357, Season14HeroPowerBatch9Event::BEGIN_TURN, state, result);
        CHECK(!result.prizeReady);
    }
    ResolveSeason14HeroPowerBatch9Event(
        67357, Season14HeroPowerBatch9Event::BEGIN_TURN, state, result);
    CHECK(result.prizeReady);
    CHECK(state.prizeChoiceReady);
    CHECK(ConsumeSeason14PrizeWall(state));
}
