// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch3.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 3 static pair")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch3::AddAll(cards);

    for (const auto* id : { "BG26_175", "BG26_175_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetStartCombatTask().empty());
        CHECK(cards.at(id).power.GetDeathrattleTask().empty());
    }
}
