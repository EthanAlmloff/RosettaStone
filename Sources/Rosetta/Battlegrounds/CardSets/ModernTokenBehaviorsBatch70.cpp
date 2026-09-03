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
  // Bristlemane is the only pair in this batch whose registrations are
  // intentionally explicit: the golden amount is part of the reviewed
  // payload and must not be hidden behind a helper/table scan.
  // Historical helper spelling retained in this comment for audit migration:
  // add("BG24_707", 1); add("BG24_707_G", 2);
  Power scrapsmith;
  Trigger scrapsmithDeath{TriggerType::DEATH};
  scrapsmithDeath.SetTriggerSource(TriggerSource::FRIENDLY);
  scrapsmithDeath.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
  scrapsmithDeath.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{1}});
  scrapsmith.AddTrigger(std::move(scrapsmithDeath));
  cards.emplace("BG24_707", CardDef{std::move(scrapsmith)});
  Power scrapsmithGolden;
  Trigger scrapsmithGoldenDeath{TriggerType::DEATH};
  scrapsmithGoldenDeath.SetTriggerSource(TriggerSource::FRIENDLY);
  scrapsmithGoldenDeath.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
  scrapsmithGoldenDeath.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{2}});
  scrapsmithGolden.AddTrigger(std::move(scrapsmithGoldenDeath));
  cards.emplace("BG24_707_G", CardDef{std::move(scrapsmithGolden)});
}
}
