#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeTavernForFriendlyDemonsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackGainHealthTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnGoldenCountSelfBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch67::AddAll(std::map<std::string, CardDef>& cards) {
  // Famished Felbat: each friendly Demon consumes one Tavern minion at turn end.
  Power normal;
  Trigger normalEnd{TriggerType::TURN_END};
  normalEnd.SetTriggerSource(TriggerSource::FRIENDLY);
  normalEnd.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DEMON); }});
  normalEnd.SetTasks({SimpleTasks::ConsumeTavernForFriendlyDemonsTask{1}});
  normal.AddTrigger(std::move(normalEnd));
  cards.emplace("BG21_005", CardDef{std::move(normal)});
  Power golden;
  Trigger goldenEnd{TriggerType::TURN_END};
  goldenEnd.SetTriggerSource(TriggerSource::FRIENDLY);
  goldenEnd.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DEMON); }});
  goldenEnd.SetTasks({SimpleTasks::ConsumeTavernForFriendlyDemonsTask{2}});
  golden.AddTrigger(std::move(goldenEnd));
  cards.emplace("BG21_005_G", CardDef{std::move(golden)});
  Power grubber;
  Trigger grubberEnd{TriggerType::TURN_END};
  grubberEnd.SetTriggerSource(TriggerSource::SELF);
  grubberEnd.SetTasks({SimpleTasks::EndTurnGoldenCountSelfBuffTask{2}});
  grubber.AddTrigger(std::move(grubberEnd));
  cards.emplace("BGS_066", CardDef{std::move(grubber)});
  Power goldenGrubber;
  Trigger goldenGrubberEnd{TriggerType::TURN_END};
  goldenGrubberEnd.SetTriggerSource(TriggerSource::SELF);
  goldenGrubberEnd.SetTasks({SimpleTasks::EndTurnGoldenCountSelfBuffTask{4}});
  goldenGrubber.AddTrigger(std::move(goldenGrubberEnd));
  cards.emplace("TB_BaconUps_130", CardDef{std::move(goldenGrubber)});
  Power lightfang;
  Trigger lightfangEnd{TriggerType::TURN_END};
  lightfangEnd.SetTriggerSource(TriggerSource::SELF);
  lightfangEnd.SetTasks({SimpleTasks::OnePerTypeRallyBuffTask{2, 2, 1}});
  lightfang.AddTrigger(std::move(lightfangEnd));
  cards.emplace("BGS_009", CardDef{std::move(lightfang)});
  Power goldenLightfang;
  Trigger goldenLightfangEnd{TriggerType::TURN_END};
  goldenLightfangEnd.SetTriggerSource(TriggerSource::SELF);
  goldenLightfangEnd.SetTasks({SimpleTasks::OnePerTypeRallyBuffTask{4, 4, 1}});
  goldenLightfang.AddTrigger(std::move(goldenLightfangEnd));
  cards.emplace("TB_BaconUps_082", CardDef{std::move(goldenLightfang)});
  Power whelp;
  Trigger whelpGain{TriggerType::GAIN_ATTACK};
  whelpGain.SetTriggerSource(TriggerSource::FRIENDLY);
  whelpGain.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DRAGON); }});
  whelpGain.SetTasks({SimpleTasks::AttackGainHealthTask{1, Race::DRAGON}});
  whelp.AddTrigger(std::move(whelpGain));
  cards.emplace("BG21_013", CardDef{std::move(whelp)});
  Power goldenWhelp;
  Trigger goldenWhelpGain{TriggerType::GAIN_ATTACK};
  goldenWhelpGain.SetTriggerSource(TriggerSource::FRIENDLY);
  goldenWhelpGain.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DRAGON); }});
  goldenWhelpGain.SetTasks({SimpleTasks::AttackGainHealthTask{2, Race::DRAGON}});
  goldenWhelp.AddTrigger(std::move(goldenWhelpGain));
  cards.emplace("BG21_013_G", CardDef{std::move(goldenWhelp)});
}
}
