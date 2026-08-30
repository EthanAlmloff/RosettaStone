#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch37.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BuyMinionTask.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch37] - after-buy identity and fixed scaling") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch37::AddAll(cards);
  for (const auto* id : {"TB_BaconShop_HERO_01_Buddy_G", "BG20_HERO_102_Buddy_G", "BG34_950", "BG34_950_G"}) {
    REQUIRE(cards.contains(id)); REQUIRE(cards.at(id).power.GetTrigger());
    CHECK(cards.at(id).power.GetTrigger()->GetTriggerType() == TriggerType::BUY_MINION);
    CHECK(cards.at(id).power.GetTrigger()->GetTriggerSource() == TriggerSource::FRIENDLY);
    REQUIRE(cards.at(id).power.GetTrigger()->GetTasks().size() == 1);
    CHECK(std::holds_alternative<SimpleTasks::BuyMinionTask>(cards.at(id).power.GetTrigger()->GetTasks().front()));
  }
}
