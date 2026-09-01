#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch61.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/NomiElementalTavernBuffTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch61::AddAll(std::map<std::string, CardDef>& cards) {
  Power normal;
  Trigger played{TriggerType::AFTER_PLAY_MINION};
  played.SetTriggerSource(TriggerSource::FRIENDLY);
  played.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::ELEMENTAL); }});
  // The pinned 36.4 text is +4/+4 per Elemental, permanently.  The task
  // updates both the current Tavern offers and the persistent modifier used
  // when later offers (including generated/pool offers) are created.
  played.SetTasks({SimpleTasks::NomiElementalTavernBuffTask{4}});
  normal.AddTrigger(std::move(played));
  cards.emplace("BGS_104", CardDef{std::move(normal)});
  Power golden;
  Trigger goldenPlayed{TriggerType::AFTER_PLAY_MINION};
  goldenPlayed.SetTriggerSource(TriggerSource::FRIENDLY);
  goldenPlayed.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::ELEMENTAL); }});
  goldenPlayed.SetTasks({SimpleTasks::NomiElementalTavernBuffTask{8}});
  golden.AddTrigger(std::move(goldenPlayed));
  cards.emplace("TB_BaconUps_201", CardDef{std::move(golden)});
}
}
