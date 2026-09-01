// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[GeneratedChoices] - pinned quest reward option pool")
{
    constexpr std::array ids = {89449, 89473, 89481, 89483,
                                89947, 90861, 90865, 90914,
                                90916, 90917, 92542, 92551};
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
    CHECK(Cards::FindCardByID("BG24_Reward_129").dbfID == 90916);
    CHECK(Cards::FindCardByID("BG24_Reward_130").dbfID == 90917);
    CHECK(Cards::FindCardByID("BG24_Reward_131").dbfID == 92542);
    CHECK(Cards::FindCardByID("BG24_Reward_134").dbfID == 92551);
    for (const auto dbfID : {89449, 89473, 89481, 89483, 89947, 90861,
                             90865, 90914, 90916, 92542})
        CHECK(IsExecutableSeason14GeneratedQuestReward(dbfID));
    for (const auto dbfID : {90917, 92551})
        CHECK_FALSE(IsExecutableSeason14GeneratedQuestReward(dbfID));
    CHECK(GeneratedQuestRewardMissingLinkReason(90917) ==
          "placeholder_copy_target_0");
    CHECK(GeneratedQuestRewardMissingLinkReason(92551) ==
          "placeholder_random_card_92");

    Season14State state;
    CHECK(state.ApplyGeneratedQuestReward(89473));
    CHECK(state.HasGeneratedRewardStolenGold());
    CHECK(state.ApplyGeneratedQuestReward(89947));
    CHECK(state.HasGeneratedRewardParasol());
    CHECK(state.ApplyGeneratedQuestReward(90865));
    CHECK(state.GeneratedRewardGlobalAttack() == 4);
    CHECK(state.ApplyGeneratedQuestReward(90914));
    CHECK(state.HasGeneratedRewardMirrorShield());
    CHECK(state.ApplyGeneratedQuestReward(89449));
    CHECK(state.HasGeneratedRewardSnickerSnacks());
    CHECK(state.ApplyGeneratedQuestReward(89481));
    CHECK(state.HasGeneratedRewardEvilTwin());
    CHECK(state.ApplyGeneratedQuestReward(89483));
    CHECK(state.HasGeneratedRewardRitualDagger());
    CHECK(state.ApplyGeneratedQuestReward(90861));
    CHECK(state.HasGeneratedRewardExquisiteConch());
    CHECK(state.ApplyGeneratedQuestReward(90916));
    CHECK(state.HasGeneratedRewardSecretSinstone());
    CHECK(state.ApplyGeneratedQuestReward(92542));
    CHECK(state.HasGeneratedRewardRedHand());
    CHECK_FALSE(state.ApplyGeneratedQuestReward(90917));
}
