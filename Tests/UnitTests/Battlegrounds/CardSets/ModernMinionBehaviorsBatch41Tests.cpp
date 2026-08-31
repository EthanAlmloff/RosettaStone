#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch41.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch41] - exact random rally rows") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch41::AddAll(cards);
    CHECK(cards.size() == 4);
    CHECK(cards.contains("BG36_204")); CHECK(cards.contains("BG36_204_G"));
    CHECK(cards.contains("BG36_242")); CHECK(cards.contains("BG36_242_G"));
}
