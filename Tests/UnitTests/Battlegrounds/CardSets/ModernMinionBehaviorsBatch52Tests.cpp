#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch52.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch52] - Deft Deserter Activate") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch52::AddAll(cards);
    REQUIRE(cards.size() == 2);
    const auto normal = cards.at("BG36_621").power.GetActivate();
    const auto golden = cards.at("BG36_621_G").power.GetActivate();
    REQUIRE(normal.has_value());
    REQUIRE(golden.has_value());
    CHECK(normal->effect == ActivateEffect::TAVERN_STATS_RANDOM_KEYWORD);
    CHECK(normal->cost == 1);
    CHECK(normal->attack == 8);
    CHECK(normal->health == 8);
    CHECK(golden->attack == 16);
    CHECK(golden->health == 16);
}
