#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch22.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CounterBuffTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch22] - Magmaloc counter scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch22::AddAll(cards);
    CHECK_EQ(cards.size(), 4);
    for (const auto& row : { std::pair{ "BG25_046", 1 }, std::pair{ "BG25_046_G", 2 } })
    {
        REQUIRE(cards.contains(row.first));
        const auto& trigger = cards.at(row.first).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::TURN_END);
        REQUIRE_EQ(trigger->GetTasks().size(), 1);
        const auto* task = std::get_if<SimpleTasks::CounterBuffTask>(&trigger->GetTasks().front());
        REQUIRE(task != nullptr);
        CHECK_EQ(task->Attack(), row.second);
        CHECK_EQ(task->Health(), row.second);
        CHECK_FALSE(task->UsesBattlecries());
    }
    for (const auto& row : { std::pair{ "BG34_Giant_206", 2 },
                             std::pair{ "BG34_Giant_206_G", 4 } })
    {
        REQUIRE(cards.contains(row.first));
        const auto& trigger = cards.at(row.first).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::TURN_END);
        REQUIRE_EQ(trigger->GetTasks().size(), 1);
        const auto* task = std::get_if<SimpleTasks::CounterBuffTask>(
            &trigger->GetTasks().front());
        REQUIRE(task != nullptr);
        CHECK_EQ(task->Attack(), row.second);
        CHECK_EQ(task->Health(), row.second);
        CHECK(task->UsesBattlecries());
    }
}
