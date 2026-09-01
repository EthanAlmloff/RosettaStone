#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch57.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch57] - Tad sell variants")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch57::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.contains("BG22_202"));
    CHECK(cards.contains("BG22_202_G"));
    const auto& normal = cards.at("BG22_202").power.GetTrigger().value();
    const auto& golden = cards.at("BG22_202_G").power.GetTrigger().value();
    CHECK(normal.GetTriggerType() == TriggerType::SELL_MINION);
    CHECK(golden.GetTriggerType() == TriggerType::SELL_MINION);
    const auto& normalTask = std::get<SimpleTasks::RandomCardToHandTask>(
        normal.GetTasks().front());
    const auto& goldenTask = std::get<SimpleTasks::RandomCardToHandTask>(
        golden.GetTasks().front());
    CHECK(normalTask.GetAmount() == 1);
    CHECK(goldenTask.GetAmount() == 2);
    CHECK(normalTask.GetRace() == Race::MURLOC);
    CHECK(goldenTask.GetRace() == Race::MURLOC);
}
