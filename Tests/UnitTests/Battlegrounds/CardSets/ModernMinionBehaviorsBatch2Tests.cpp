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

    CHECK(cards.empty());
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
