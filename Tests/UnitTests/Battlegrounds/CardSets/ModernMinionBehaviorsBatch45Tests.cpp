#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch45.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch45] - exact avenge rows") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch45::AddAll(cards);
    CHECK(cards.size() == 2);
    CHECK(cards.contains("BG31_835")); CHECK(cards.contains("BG31_835_G"));
}
