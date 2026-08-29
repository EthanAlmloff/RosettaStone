// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 2 families")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch2::AddAll(cards);

    for (const auto* id : { "BG30_125", "BG30_125_G", "BG32_172",
                            "BG32_172_G", "BG35_604", "BG35_604_G",
                            "BG19_010", "BG19_010_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK_EQ(cards.at(id).power.GetDeathrattleTask().size(), 1);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 2 static pair")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch2::AddAll(cards);

    REQUIRE(cards.contains("BG32_236"));
    REQUIRE(cards.contains("BG32_236_G"));
    CHECK(cards.at("BG32_236").power.GetDeathrattleTask().empty());
    CHECK(cards.at("BG32_236_G").power.GetDeathrattleTask().empty());
}
