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
    CHECK(Cards::FindCardByID("BG24_Reward_136").dbfID == 93069);
    CHECK(Cards::FindCardByID("BG24_Reward_312").dbfID == 92552);
    for (const auto dbfID : {89449, 89473, 89481, 89483, 89947, 90861,
                             90865, 90914, 90916, 92542, 93069, 92552})
        CHECK(IsExecutableSeason14GeneratedQuestReward(dbfID));
    for (const auto dbfID : {90917, 92551})
        CHECK(IsExecutableSeason14GeneratedQuestReward(dbfID));

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
    CHECK(state.ApplyGeneratedQuestReward(93069));
    CHECK(state.HasGeneratedRewardTinyHenchmen());
    CHECK(state.ApplyGeneratedQuestReward(92552));
    CHECK(state.HasGeneratedRewardStaffOfOrigination());
    CHECK_FALSE(state.ApplyGeneratedQuestReward(90917));
    CHECK(state.ApplyGeneratedQuestReward(89645));
    CHECK(state.HasGeneratedRewardAnimaBribe());
    CHECK(state.ApplyGeneratedQuestReward(93074));
    CHECK(state.HasGeneratedRewardVictimsSpecter());
    CHECK(state.ApplyGeneratedQuestReward(90437));
    CHECK(state.HasGeneratedRewardDevilsInDetails());
    CHECK(state.ApplyGeneratedQuestReward(95867));
    CHECK(state.HasGeneratedRewardPilferedLamps());
    CHECK(state.ApplyGeneratedQuestReward(97966));
    CHECK(state.HasGeneratedRewardKidnapSack());
    CHECK(state.ApplyGeneratedQuestReward(91992));
    CHECK(state.HasGeneratedRewardAnotherHiddenBody());
    CHECK(state.ApplyGeneratedQuestReward(90917));
    CHECK(state.HasGeneratedRewardGhastlyMask());
    CHECK(state.ApplyGeneratedQuestReward(92551));
    CHECK(state.GeneratedRewardFriendsRace() == Race::INVALID);
    CHECK(state.ApplyGeneratedQuestReward(96151));
    CHECK(state.HasGeneratedRewardUnmurloc());
}

TEST_CASE("[GeneratedChoices] - Batch 113 ALT and 323/351/352/360 identities")
{
    CHECK(Cards::FindCardByID("BG24_Reward_113_ALT").dbfID == 95858);
    CHECK(Cards::FindCardByID("BG24_Reward_323").dbfID == 96148);
    CHECK(Cards::FindCardByID("BG24_Reward_351").dbfID == 96149);
    CHECK(Cards::FindCardByID("BG24_Reward_352").dbfID == 96150);
    CHECK(Cards::FindCardByID("BG24_Reward_360").dbfID == 97436);

    CHECK(IsExecutableSeason14GeneratedQuestReward(95858));
    CHECK(IsExecutableSeason14GeneratedQuestReward(96148));
    CHECK(IsExecutableSeason14GeneratedQuestReward(96149));
    CHECK(IsExecutableSeason14GeneratedQuestReward(96150));
    CHECK(IsExecutableSeason14GeneratedQuestReward(97436));

    // ALT Ritual Dagger is intentionally not aliased to the original
    // death-buff family: both rows share a display name but have different
    // simulator contracts.
    const auto* original = FindSeason14GeneratedQuestReward(89483);
    const auto* alt = FindSeason14GeneratedQuestReward(95858);
    REQUIRE(original != nullptr);
    REQUIRE(alt != nullptr);
    CHECK(original->effect != alt->effect);

    Season14State state;
    CHECK(state.ApplyGeneratedQuestReward(95858));
    CHECK(state.HasGeneratedRewardRitualDaggerRepeat());
    CHECK_FALSE(state.HasGeneratedRewardRitualDagger());
    CHECK(state.ApplyGeneratedQuestReward(96148));
    CHECK(state.HasGeneratedRewardNineLives());
    CHECK(state.ApplyGeneratedQuestReward(96149));
    CHECK(state.HasGeneratedRewardTotemicTavern());
    CHECK(state.ApplyGeneratedQuestReward(96150));
    CHECK(state.HasGeneratedRewardPurifiedShard());
    CHECK(state.ApplyGeneratedQuestReward(97436));
    CHECK(state.HasGeneratedRewardTheWall());
}

TEST_CASE("[GeneratedChoices] - typed lifecycle reward ownership")
{
    // These rewards are resolved by Player/Season14 lifecycle hooks rather
    // than CardDef powers. Keep their DBF-to-state ownership explicit so the
    // coverage audit can credit the real runtime path.
    CHECK(Cards::FindCardByID("BG27_Reward_502").dbfID == 104697);
    CHECK(Cards::FindCardByID("BG27_Reward_503").dbfID == 104703);
    CHECK(Cards::FindCardByID("BG27_Reward_504").dbfID == 104724);
    CHECK(Cards::FindCardByID("BG27_Reward_804").dbfID == 104675);
    CHECK(Cards::FindCardByID("BG28_Reward_509").dbfID == 110309);

    Season14State state;
    CHECK(state.ApplyGeneratedQuestReward(104697));
    CHECK(state.HasGeneratedRewardBoomSquad());
    CHECK(state.ApplyGeneratedQuestReward(104703));
    CHECK(state.HasGeneratedRewardInvigoratingConch());
    CHECK(state.ApplyGeneratedQuestReward(104724));
    CHECK(state.HasGeneratedRewardTimelineAcceleration());
    CHECK(state.ApplyGeneratedQuestReward(104675));
    CHECK(state.HasGeneratedRewardSturdyShard());
    CHECK(state.ApplyGeneratedQuestReward(110309));
    CHECK(state.HasGeneratedRewardSmeltingChamber());
    CHECK(Cards::FindCardByID("BG28_Reward_504").dbfID == 110303);
    CHECK(Cards::FindCardByID("BG27_Reward_803").dbfID == 104670);
    CHECK(Cards::FindCardByID("BG28_Reward_518").dbfID == 110551);
    CHECK(state.ApplyGeneratedQuestReward(110303));
    CHECK(state.HasGeneratedRewardCycleEnergy());
    CHECK(state.ApplyGeneratedQuestReward(104670));
    CHECK(state.HasGeneratedRewardTurbulentTombs());
    CHECK(state.ApplyGeneratedQuestReward(110551));
    CHECK(state.HasGeneratedRewardStableAmalgamation());
}

TEST_CASE("[GeneratedChoices] - Yogg wheel and Gilnean War Horn lifecycle state")
{
    CHECK(Cards::FindCardByID("BG24_Reward_135").dbfID == 92563);
    CHECK(Cards::FindCardByID("BG27_Reward_802").dbfID == 104673);
    CHECK(IsExecutableSeason14GeneratedQuestReward(92563));
    CHECK(IsExecutableSeason14GeneratedQuestReward(104673));

    Season14State state;
    CHECK(state.ApplyGeneratedQuestReward(92563));
    CHECK(state.HasGeneratedRewardYoggTasties());
    CHECK(state.ApplyGeneratedQuestReward(104673));
    CHECK(state.HasGeneratedRewardBattlecryRepeat());
    CHECK(state.GeneratedRewardBattlecryMinionDbfIDs().empty());
    CHECK(state.GeneratedRewardYoggTastiesCount() == 1);
    CHECK(state.GeneratedRewardBattlecryRepeatCount() == 1);
    CHECK(state.ApplyGeneratedQuestReward(92563));
    CHECK(state.GeneratedRewardYoggTastiesCount() == 2);
    CHECK(state.ApplyGeneratedQuestReward(104673));
    CHECK(state.GeneratedRewardBattlecryRepeatCount() == 2);
    CHECK(state.GeneratedRewardYoggOutcomes().empty());
}
