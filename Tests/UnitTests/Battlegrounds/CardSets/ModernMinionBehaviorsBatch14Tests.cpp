#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch14.hpp>
#include <map>
#include <string>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[ModernMinionBehaviorsBatch14] - duplicate ownership fails closed")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch14::AddAll(cards);

    CHECK(cards.empty());
}
