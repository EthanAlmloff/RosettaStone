#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch26.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch26] - deterministic target-free Activate pairs")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch26::AddAll(cards);
    CHECK_EQ(cards.size(), 4);
    for (const auto id : {"BG36_509", "BG36_509_G", "BG36_346", "BG36_346_G"})
        CHECK(cards.contains(id));
    CHECK(cards.at("BG36_509").power.GetActivate()->effect == ActivateEffect::GAIN_GOLD);
    CHECK(cards.at("BG36_509").power.GetActivate()->cost == 1);
    CHECK(cards.at("BG36_509").power.GetActivate()->nextTurn);
    CHECK(cards.at("BG36_509_G").power.GetActivate()->amount == 6);
    CHECK(cards.at("BG36_509_G").power.GetActivate()->cost == 1);
    CHECK(cards.at("BG36_346").power.GetActivate()->cardID == "BG28_897");
    CHECK(cards.at("BG36_346").power.GetActivate()->cost == 1);
    CHECK(cards.at("BG36_346_G").power.GetActivate()->amount == 4);
    CHECK(cards.at("BG36_346_G").power.GetActivate()->cost == 1);
}
