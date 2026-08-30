// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

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
