#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch42.hpp>

#include <doctest/doctest.h>
#include <map>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch42] - Fodder refresh counts")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch42::AddAll(cards);
    CHECK(cards.contains("BG36_730"));
    CHECK(cards.contains("BG36_730_G"));
    CHECK(cards.size() == 2);
}
