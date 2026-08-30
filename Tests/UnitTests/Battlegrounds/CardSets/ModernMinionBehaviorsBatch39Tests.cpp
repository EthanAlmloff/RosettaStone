#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch39.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch39] - fixed resource normal/golden families") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch39::AddAll(cards);
  CHECK(cards.empty());
}
