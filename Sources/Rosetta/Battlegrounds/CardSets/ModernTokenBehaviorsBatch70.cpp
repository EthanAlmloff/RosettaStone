#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch70.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch70::AddAll(std::map<std::string, CardDef>& cards) {
  auto add = [&cards](const char* id, int amount) {
    Power power; Trigger trigger{TriggerType::DEATH};
    trigger.SetTriggerSource(TriggerSource::FRIENDLY);
    trigger.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
    trigger.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{amount}});
    power.AddTrigger(std::move(trigger)); cards.emplace(id, CardDef{std::move(power)});
  };
  add("BG24_707", 1); add("BG24_707_G", 2);
}
}
