#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch66.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch66::AddAll(std::map<std::string, CardDef>& cards) {
  // Wildfire's combat spill is resolved by Battle::Attack, where both the
  // pre-hit health and adjacent enemy field are authoritative.
  cards.emplace("BGS_126", CardDef{});
  cards.emplace("TB_BaconUps_166", CardDef{});
}
}
