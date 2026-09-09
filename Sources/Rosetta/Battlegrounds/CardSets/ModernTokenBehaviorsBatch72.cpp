#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch72.hpp>

#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch72::AddAll(std::map<std::string, CardDef>& cards) {
  // Monstrous Macaw's premium is the linked generated token for BGS_078.
  // Keep the normal and golden payloads explicit: both trigger the same
  // left-most other friendly deathrattle, with the premium repeating it.
  Power golden;
  golden.AddRallyTask(SimpleTasks::TriggerLeftmostDeathrattleTask{2});
  cards.emplace("TB_BaconUps_135", CardDef{std::move(golden)});
}
}  // namespace RosettaStone::Battlegrounds
