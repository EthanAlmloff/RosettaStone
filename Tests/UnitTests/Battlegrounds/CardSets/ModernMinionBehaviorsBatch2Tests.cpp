// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/SewerRatTokenBehaviors.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 2 families")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch2::AddAll(cards);

    CHECK(cards.contains("BG19_010t"));
    CHECK(cards.contains("BG19_010_Gt"));
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

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - Sewer Rat token family")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch2::AddAll(cards);
    CHECK(cards.at("BG19_010t").power.GetDeathrattleTask().empty());
    CHECK(cards.at("BG19_010_Gt").power.GetDeathrattleTask().empty());
    CHECK(FindSewerRatToken(70791)->health == 3);
    CHECK(FindSewerRatToken(70802)->health == 6);
}
