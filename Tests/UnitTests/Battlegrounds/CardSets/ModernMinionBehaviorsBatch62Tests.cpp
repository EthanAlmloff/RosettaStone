#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch62.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonRecentDeadMinionsTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch62] - Kangor")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch62::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.contains("BGS_012"));
    CHECK(cards.contains("TB_BaconUps_087"));
    const auto& normal = std::get<SimpleTasks::SummonRecentDeadMinionsTask>(
        cards.at("BGS_012").power.GetDeathrattleTask().front());
    const auto& golden = std::get<SimpleTasks::SummonRecentDeadMinionsTask>(
        cards.at("TB_BaconUps_087").power.GetDeathrattleTask().front());
    CHECK(normal.RaceFilter() == Race::MECHANICAL);
    CHECK(normal.Count() == 2);
    CHECK(golden.RaceFilter() == Race::MECHANICAL);
    CHECK(golden.Count() == 4);
}
