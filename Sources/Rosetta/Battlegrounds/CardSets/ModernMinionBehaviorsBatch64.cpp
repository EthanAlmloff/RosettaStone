#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch64.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomElementalHandAndSummonTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch64::AddAll(std::map<std::string, CardDef>& cards) {
  Power normal; normal.AddDeathrattleTask(SimpleTasks::RandomElementalHandAndSummonTask{1});
  cards.emplace("BGS_121", CardDef{std::move(normal)});
  Power golden; golden.AddDeathrattleTask(SimpleTasks::RandomElementalHandAndSummonTask{2});
  cards.emplace("TB_BaconUps_165", CardDef{std::move(golden)});
}
}
