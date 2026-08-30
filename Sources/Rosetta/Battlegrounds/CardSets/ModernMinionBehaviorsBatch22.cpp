#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch22.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CounterBuffTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch22::AddAll(std::map<std::string, CardDef>& cards)
{
    for (const auto& spec : { std::pair{ "BG25_046", 1 }, std::pair{ "BG25_046_G", 2 } })
    {
        Power power;
        Trigger trigger{ TriggerType::TURN_END };
        trigger.SetTriggerSource(TriggerSource::SELF);
        trigger.SetTasks(std::vector<TaskType>{ SimpleTasks::CounterBuffTask{ spec.second, spec.second, false } });
        power.AddTrigger(std::move(trigger));
        cards.emplace(spec.first, CardDef{ std::move(power) });
    }
    for (const auto& spec : { std::pair{ "BG34_Giant_206", 2 },
                              std::pair{ "BG34_Giant_206_G", 4 } })
    {
        Power power;
        Trigger trigger{ TriggerType::TURN_END };
        trigger.SetTriggerSource(TriggerSource::SELF);
        trigger.SetTasks(std::vector<TaskType>{
            SimpleTasks::CounterBuffTask{ spec.second, spec.second, true } });
        power.AddTrigger(std::move(trigger));
        cards.emplace(spec.first, CardDef{ std::move(power) });
    }
}
}
