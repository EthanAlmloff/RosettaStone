#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch58.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch58] - consume variants")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch58::AddAll(cards);
    REQUIRE(cards.size() == 4);
    for (const auto id : {"BG21_004", "BG21_004_G", "BG23_357", "BG23_357_G"})
        CHECK(cards.contains(id));
    CHECK(std::get<SimpleTasks::ConsumeRandomTavernTask>(
              cards.at("BG21_004").power.GetTrigger()->GetTasks().front())
              .Multiplier() == 1);
    CHECK(std::get<SimpleTasks::ConsumeRandomTavernTask>(
              cards.at("BG21_004_G").power.GetTrigger()->GetTasks().front())
              .Multiplier() == 2);
    CHECK(std::get<SimpleTasks::ConsumeRandomTavernTask>(
              cards.at("BG23_357").power.GetBattlecryTask().front())
              .Multiplier() == 1);
    CHECK(std::get<SimpleTasks::ConsumeRandomTavernTask>(
              cards.at("BG23_357_G").power.GetBattlecryTask().front())
              .Multiplier() == 2);
}
