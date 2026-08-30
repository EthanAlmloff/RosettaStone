#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch32.hpp>
namespace RosettaStone::Battlegrounds {
namespace {
void Static(std::map<std::string, CardDef>& c, const char* id) { c.emplace(id, CardDef{}); }
}
void ModernMinionBehaviorsBatch32::AddAll(std::map<std::string, CardDef>& c) {
  // Static keyword entities are intentionally empty: CardLoader owns their exact tags.
  Static(c, "BGS_034"); Static(c, "TB_BaconUps_149");
}
}
