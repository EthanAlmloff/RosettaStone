#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch58.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch58::AddAll(std::map<std::string, CardDef>& cards) {
  // Ur'zul consumes one Tavern minion for each played friendly Demon.  The
  // source condition prevents unrelated friendly plays from firing it.
  Power normal;
  Trigger played{TriggerType::AFTER_PLAY_MINION};
  played.SetTriggerSource(TriggerSource::FRIENDLY);
  played.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DEMON); }});
  played.SetTasks({SimpleTasks::ConsumeRandomTavernTask{1}});
  normal.AddTrigger(std::move(played));
  cards.emplace("BG21_004", CardDef{std::move(normal)});
  Power golden;
  Trigger goldenPlayed{TriggerType::AFTER_PLAY_MINION};
  goldenPlayed.SetTriggerSource(TriggerSource::FRIENDLY);
  goldenPlayed.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DEMON); }});
  goldenPlayed.SetTasks({SimpleTasks::ConsumeRandomTavernTask{2}});
  golden.AddTrigger(std::move(goldenPlayed));
  cards.emplace("BG21_004_G", CardDef{std::move(golden)});

  // Mind Muck supplies the selected friendly Demon as Trigger::Run's target;
  // the task rejects non-Demons before mutating either zone.
  Power muck;
  muck.AddBattlecryTask(SimpleTasks::ConsumeRandomTavernTask{1});
  cards.emplace("BG23_357", CardDef{std::move(muck)});
  Power goldenMuck;
  goldenMuck.AddBattlecryTask(SimpleTasks::ConsumeRandomTavernTask{2});
  cards.emplace("BG23_357_G", CardDef{std::move(goldenMuck)});
}
}
