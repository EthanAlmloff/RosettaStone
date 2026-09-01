#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch62.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonRecentDeadMinionsTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch62::AddAll(std::map<std::string, CardDef>& cards) {
  Power normal; normal.AddDeathrattleTask(SimpleTasks::SummonRecentDeadMinionsTask{Race::MECHANICAL, 2});
  cards.emplace("BGS_012", CardDef{std::move(normal)});
  Power golden; golden.AddDeathrattleTask(SimpleTasks::SummonRecentDeadMinionsTask{Race::MECHANICAL, 4});
  cards.emplace("TB_BaconUps_087", CardDef{std::move(golden)});
}
}
