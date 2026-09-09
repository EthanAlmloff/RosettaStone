#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch75.hpp>

#include <Rosetta/Battlegrounds/Enchants/Power.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch75::AddAll(
    std::map<std::string, CardDef>& cards) {
  // Mothership is a generated Avenge token.  Its normal form adds one
  // random Protoss; premium adds two.  Protoss eligibility and hand-full
  // handling are delegated to the same canonical helper used by Warp Gate.
  Power normal;
  normal.AddAvenge({AvengeEffect::ADD_RANDOM_PROTOSS, 4, 0, 0,
                    Race::INVALID, false, "", 1});
  cards.emplace("BG31_HERO_802pt7", CardDef{std::move(normal)});

  Power golden;
  golden.AddAvenge({AvengeEffect::ADD_RANDOM_PROTOSS, 4, 0, 0,
                    Race::INVALID, false, "", 2});
  cards.emplace("BG31_HERO_802pt7_G", CardDef{std::move(golden)});
}
}  // namespace RosettaStone::Battlegrounds
