#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch63.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DestroyLastDamageSourceTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch63::AddAll(std::map<std::string, CardDef>& cards) {
  Power normal; normal.AddDeathrattleTask(SimpleTasks::DestroyLastDamageSourceTask{});
  cards.emplace("BG23_318", CardDef{std::move(normal)});
  Power golden; golden.AddDeathrattleTask(SimpleTasks::DestroyLastDamageSourceTask{});
  cards.emplace("BG23_318_G", CardDef{std::move(golden)});
}
}
