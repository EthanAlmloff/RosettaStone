#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch74.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HealthGainHealthTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch74::AddAll(
    std::map<std::string, CardDef>& cards) {
  auto makeTitanicGuardian = [](int multiplier) {
    Power power;
    Trigger trigger{TriggerType::GAIN_HEALTH};
    trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
    trigger.SetCondition(SelfCondition{[](Minion& source) {
      return !source.IsDestroyed();
    }});
    trigger.SetTasks({SimpleTasks::HealthGainHealthTask{multiplier}});
    power.AddTrigger(std::move(trigger));
    return CardDef{std::move(power)};
  };

  // Titanic Guardian: whenever another friendly minion gains Health, gain
  // that much Health. Golden doubles the copied amount.
  cards.emplace("TB_BaconShop_HERO_39_Buddy", makeTitanicGuardian(1));
  cards.emplace("TB_BaconShop_HERO_39_Buddy_G", makeTitanicGuardian(2));
}
}  // namespace RosettaStone::Battlegrounds
