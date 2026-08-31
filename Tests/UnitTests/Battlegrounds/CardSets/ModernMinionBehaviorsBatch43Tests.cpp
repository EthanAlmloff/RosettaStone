#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch43.hpp>

#include <doctest/doctest.h>
#include <map>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch43] - Deathstrider rally")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch43::AddAll(cards);
    CHECK(cards.contains("BG36_208"));
    CHECK(cards.contains("BG36_208_G"));
    CHECK(cards.size() == 2);
}
