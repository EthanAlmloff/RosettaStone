#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch59.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch59::AddAll(std::map<std::string, CardDef>& cards) {
  Power normal;
  Trigger sold{TriggerType::SELL_MINION};
  sold.SetTriggerSource(TriggerSource::SELF);
  sold.SetTasks({SimpleTasks::MinionOfferingTask{Race::INVALID, 1, 1, 3}});
  normal.AddTrigger(std::move(sold));
  cards.emplace("BG24_715", CardDef{std::move(normal)});
  Power golden;
  Trigger goldenSold{TriggerType::SELL_MINION};
  goldenSold.SetTriggerSource(TriggerSource::SELF);
  goldenSold.SetTasks({SimpleTasks::MinionOfferingTask{Race::INVALID, 1, 1, 3}});
  golden.AddTrigger(std::move(goldenSold));
  cards.emplace("BG24_715_G", CardDef{std::move(golden)});
}
}
