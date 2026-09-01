#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch60.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch60::AddAll(std::map<std::string, CardDef>& cards) {
  // Titus is a board aura; deathrattle repetition is resolved centrally by
  // Minion::ActivateTask so every existing and future deathrattle receives the
  // same normal (one extra) / golden (two extra) treatment.
  cards.emplace("BG25_354", CardDef{});
  cards.emplace("BG25_354_G", CardDef{});
}
}
