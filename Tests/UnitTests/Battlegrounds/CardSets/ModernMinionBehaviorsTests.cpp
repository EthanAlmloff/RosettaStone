// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - normal and golden families")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);

    CHECK(cards.contains("BG31_803"));
    CHECK(cards.contains("BG31_803_G"));
    CHECK(cards.contains("BG29_611"));
    CHECK(cards.contains("BG29_611_G"));
    CHECK(cards.contains("BG28_300"));
    CHECK(cards.contains("BG28_300_G"));

    CHECK_EQ(cards.at("BG31_803").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG31_803_G").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG29_611").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG29_611_G").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG28_300").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG28_300_G").power.GetDeathrattleTask().size(), 1);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - static keyword registrations")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);

    // Static keywords come from CardLoader metadata, so these definitions
    // intentionally contain no task chain while still satisfying the pool's
    // explicit behavior-registration contract.
    for (const auto* id : { "BGS_119", "BGS_131", "BG_BOT_911" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetDeathrattleTask().empty());
        CHECK(cards.at(id).power.GetStartCombatTask().empty());
    }
}
