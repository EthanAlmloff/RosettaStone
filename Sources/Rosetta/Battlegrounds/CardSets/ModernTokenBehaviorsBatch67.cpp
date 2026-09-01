#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch67::AddAll(std::map<std::string, CardDef>& cards) {
  // Famished Felbat: each friendly Demon consumes one Tavern minion at turn end.
  for (const auto& [id, amount] : {std::pair{"BG21_005", 1}, std::pair{"BG21_005_G", 2}}) {
    Power p;
    Trigger end{TriggerType::TURN_END};
    end.SetTriggerSource(TriggerSource::FRIENDLY);
    end.SetCondition(SelfCondition{[](Minion& source) { return source.HasRace(Race::DEMON); }});
    end.SetTasks({SimpleTasks::ConsumeRandomTavernTask{amount}});
    p.AddTrigger(std::move(end));
    cards.emplace(id, CardDef{std::move(p)});
  }
}
}
