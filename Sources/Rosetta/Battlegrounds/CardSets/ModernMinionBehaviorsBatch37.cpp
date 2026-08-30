#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch37.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BuyMinionTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch37::AddAll(std::map<std::string, CardDef>& cards) {
  // SI:7 Scout (golden buddy): after buying any card, gain +4/+4.
  {
    Power p; Trigger t{TriggerType::BUY_MINION};
    t.SetTriggerSource(TriggerSource::FRIENDLY);
    t.SetTasks({SimpleTasks::BuyMinionTask{4, 4}});
    p.AddTrigger(std::move(t));
    cards.emplace("TB_BaconShop_HERO_01_Buddy_G", CardDef{std::move(p)});
  }
  // Dranosh Saurfang (golden buddy): gain the bought minion's current stats.
  {
    Power p; Trigger t{TriggerType::BUY_MINION};
    t.SetTriggerSource(TriggerSource::FRIENDLY);
    t.SetTasks({SimpleTasks::BuyMinionTask{0, 0, 1}});
    p.AddTrigger(std::move(t));
    cards.emplace("BG20_HERO_102_Buddy_G", CardDef{std::move(p)});
  }
  // Stone Age Slab: once per turn, the bought minion receives +20/+20 and its
  // current stats are doubled (tripled for golden).  The per-source counter
  // is reset at recruit start and consumed only after a successful buy.
  {
    Power p; Trigger t{TriggerType::BUY_MINION};
    t.SetTriggerSource(TriggerSource::FRIENDLY);
    t.SetTasks({SimpleTasks::BuyMinionTask{20, 20, 2, true, 1}});
    p.AddTrigger(std::move(t));
    cards.emplace("BG34_950", CardDef{std::move(p)});
  }
  {
    Power p; Trigger t{TriggerType::BUY_MINION};
    t.SetTriggerSource(TriggerSource::FRIENDLY);
    t.SetTasks({SimpleTasks::BuyMinionTask{20, 20, 3, true, 1}});
    p.AddTrigger(std::move(t));
    cards.emplace("BG34_950_G", CardDef{std::move(p)});
  }
}
}
