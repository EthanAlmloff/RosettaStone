#include <Rosetta/Battlegrounds/Models/Season14.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14] - Public decisions validate and clear")
{
    Season14State state;
    state.BeginDecision(
        Season14Decision::DISCOVER,
        {Season14Offering{101, 1}, Season14Offering{102, 2}});

    CHECK(state.pendingDecision == Season14Decision::DISCOVER);
    CHECK(state.choiceOfferings.size() == 2);
    CHECK(!state.SelectDecision(2));
    CHECK(state.pendingDecision == Season14Decision::DISCOVER);
    CHECK(state.SelectDecision(1));
    CHECK(state.pendingDecision == Season14Decision::NONE);
    CHECK(state.pendingOfferings.empty());
}

TEST_CASE("[Season14] - Hero power availability is costed and one-shot")
{
    Season14State state;
    state.SetHeroPower(9001, 2, true);

    CHECK(!state.CanUseHeroPower(1));
    CHECK(state.CanUseHeroPower(2));
    CHECK(state.UseHeroPower());
    CHECK(!state.CanUseHeroPower(20));
    CHECK(!state.UseHeroPower());
}

TEST_CASE("[Season14] - Persistent effects and event hooks")
{
    Season14State state;
    state.AddTrinket({7001, 2, true});
    state.AddDarkGift({8001, 1, true});

    CHECK(state.ConsumeEffect(state.trinkets, 0));
    CHECK(state.trinkets[0].remainingUses == 1);
    CHECK(state.ConsumeEffect(state.trinkets, 0));
    CHECK(!state.trinkets[0].active);
    CHECK(!state.ConsumeEffect(state.trinkets, 0));
    CHECK(state.ConsumeEffect(state.darkGifts, 0));
    CHECK(!state.darkGifts[0].active);

    state.Emit(Season14Event::RECRUIT_START);
    state.Emit(Season14Event::COMBAT_END);
    CHECK(state.eventCounts[0] == 1);
    CHECK(state.eventCounts[3] == 1);
    CHECK(Season14State::IsValidBoardTarget(0, 1));
    CHECK(!Season14State::IsValidBoardTarget(1, 1));
}
