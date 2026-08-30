#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch19.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddTavernCoinTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageHeroTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch19] - canonical coin lifecycle triggers")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch19::AddAll(cards);
    CHECK_EQ(cards.size(), 8);
    struct Expected { const char* id; TriggerType type; int amount; };
    constexpr Expected expected[] = {
        { "BG21_017", TriggerType::DEATH, 1 },
        { "BG21_017_G", TriggerType::DEATH, 2 },
        { "BG33_315", TriggerType::RALLY, 1 },
        { "BG33_315_G", TriggerType::RALLY, 2 },
        { "BG34_234", TriggerType::TURN_START, 2 },
        { "BG34_234_G", TriggerType::TURN_START, 4 },
        { "BG27_011", TriggerType::TURN_END, 1 },
        { "BG27_011_G", TriggerType::TURN_END, 2 },
    };
    for (const auto& row : expected)
    {
        REQUIRE(cards.contains(row.id));
        const auto& trigger = cards.at(row.id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == row.type);
        CHECK(trigger->GetTriggerSource() == TriggerSource::SELF);
        const auto& tasks = trigger->GetTasks();
        const auto expectedTasks = row.type == TriggerType::TURN_END ?
                                       static_cast<std::size_t>(row.amount * 2) :
                                       static_cast<std::size_t>(1);
        CHECK_EQ(tasks.size(), expectedTasks);
        if (row.type == TriggerType::TURN_END)
        {
            const auto expectedRepeats = row.amount;
            CHECK_EQ(tasks.size(), static_cast<std::size_t>(expectedRepeats * 2));
            for (int i = 0; i < expectedRepeats; ++i)
            {
                CHECK(std::holds_alternative<SimpleTasks::DamageHeroTask>(
                    tasks[static_cast<std::size_t>(i * 2)]));
                REQUIRE(std::holds_alternative<SimpleTasks::AddTavernCoinTask>(
                    tasks[static_cast<std::size_t>(i * 2 + 1)]));
                CHECK_EQ(std::get<SimpleTasks::AddTavernCoinTask>(
                             tasks[static_cast<std::size_t>(i * 2 + 1)])
                             .Amount(), 1);
            }
        }
        else
        {
            REQUIRE(std::holds_alternative<SimpleTasks::AddTavernCoinTask>(tasks.front()));
            CHECK_EQ(std::get<SimpleTasks::AddTavernCoinTask>(tasks.front()).Amount(), row.amount);
        }
    }
}
