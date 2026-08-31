#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch55.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch55] - Living Prison") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch55::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.at("BG36_180").power.GetActivate()->effect == ActivateEffect::GAIN_NEXT_BOUGHT_STATS);
    CHECK(cards.at("BG36_180").power.GetActivate()->amount == 1);
    CHECK(cards.at("BG36_180_G").power.GetActivate()->amount == 2);
}
