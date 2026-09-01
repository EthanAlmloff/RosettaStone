#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch64.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomElementalHandAndSummonTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch64] - Gentle Djinni")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch64::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.contains("BGS_121"));
    CHECK(cards.contains("TB_BaconUps_165"));
    CHECK(std::get<SimpleTasks::RandomElementalHandAndSummonTask>(
              cards.at("BGS_121").power.GetDeathrattleTask().front())
              .Amount() == 1);
    CHECK(std::get<SimpleTasks::RandomElementalHandAndSummonTask>(
              cards.at("TB_BaconUps_165").power.GetDeathrattleTask().front())
              .Amount() == 2);
}
