#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch16.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch16::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Accord-o-Tron: Magnetic; at the start of your turn gain 1 Gold.
    // Golden doubles the exact trigger task. Magnetic is metadata-owned and
    // is deliberately not registered as executable until magnetization rules
    // exist in the simulator.
    Power normal;
    Trigger trigger{ TriggerType::TURN_START };
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks(std::vector<TaskType>{ SimpleTasks::GainGoldTask{ 1 } });
    normal.AddTrigger(std::move(trigger));
    Power golden;
    Trigger goldenTrigger{ TriggerType::TURN_START };
    goldenTrigger.SetTriggerSource(TriggerSource::SELF);
    goldenTrigger.SetTasks(std::vector<TaskType>{ SimpleTasks::GainGoldTask{ 2 } });
    golden.AddTrigger(std::move(goldenTrigger));
}
}
