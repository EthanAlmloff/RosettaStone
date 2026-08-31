// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[GeneratedChoices] - pinned quest reward option pool")
{
    constexpr std::array ids = {89449, 89473, 89481, 89483,
                                89947, 90861, 90865, 90914};
    for (const auto dbfID : ids)
        CHECK(IsSeason14GeneratedQuestReward(dbfID));
    CHECK(Cards::FindCardByID("BG24_Reward_107").dbfID == 89449);
    CHECK(Cards::FindCardByID("BG24_Reward_109").dbfID == 89473);
    CHECK(Cards::FindCardByID("BG24_Reward_111").dbfID == 89481);
    CHECK(Cards::FindCardByID("BG24_Reward_113").dbfID == 89483);
    CHECK(Cards::FindCardByID("BG24_Reward_115").dbfID == 89947);
    CHECK(Cards::FindCardByID("BG24_Reward_123").dbfID == 90861);
    CHECK(Cards::FindCardByID("BG24_Reward_125").dbfID == 90865);
    CHECK(Cards::FindCardByID("BG24_Reward_128").dbfID == 90914);
}
