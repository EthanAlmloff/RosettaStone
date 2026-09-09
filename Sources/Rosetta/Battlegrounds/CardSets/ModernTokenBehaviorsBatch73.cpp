#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch73.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackGainHealthTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TemporarySelfStatsTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch73::AddAll(
    std::map<std::string, CardDef>& cards) {
  auto makeTentacle = [](int amount) {
    Power p;
    for (const auto triggerType : {TriggerType::GAIN_ATTACK,
                                   TriggerType::GAIN_HEALTH}) {
      Trigger trigger{triggerType};
      // The printed text says "a different friendly minion".  In
      // particular, do not let the Tentacle observe its own temporary stat
      // application: that would turn one gain into an unbounded recursive
      // chain (and is also contrary to the card text).
      trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
      trigger.SetCondition(SelfCondition{[](Minion& source) {
        return !source.IsDestroyed();
      }});
      trigger.SetTasks({SimpleTasks::TemporarySelfStatsTask{amount, amount}});
      p.AddTrigger(std::move(trigger));
    }
    return CardDef{std::move(p)};
  };
  cards.emplace("TB_BaconShop_HERO_29_Buddy", makeTentacle(1));
  cards.emplace("TB_BaconShop_HERO_29_Buddy_G", makeTentacle(2));

  auto makeSinestra = [](int amount) {
    Power p;
    Trigger trigger{TriggerType::GAIN_ATTACK};
    trigger.SetTriggerSource(TriggerSource::FRIENDLY);
    trigger.SetCondition(SelfCondition{[](Minion& source) {
      return !source.IsDestroyed();
    }});
    trigger.SetTasks({SimpleTasks::AttackGainHealthTask{amount, Race::INVALID}});
    p.AddTrigger(std::move(trigger));
    return CardDef{std::move(p)};
  };
  cards.emplace("TB_BaconShop_HERO_52_Buddy", makeSinestra(1));
  cards.emplace("TB_BaconShop_HERO_52_Buddy_G", makeSinestra(2));
}
}  // namespace RosettaStone::Battlegrounds
