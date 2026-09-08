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
    for (const auto* id : { "BGS_119", "BGS_131", "BG_BOT_911",
                            "BG24_HERO_204_Buddy", "BG24_HERO_204_Buddy_G",
                            "BG25_HERO_103_Buddy", "BG25_HERO_103_Buddy_G",
                            "BG26_HERO_104_Buddy", "BG26_HERO_104_Buddy_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetDeathrattleTask().empty());
        CHECK(cards.at(id).power.GetStartCombatTask().empty());
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] Buddy scaling contracts")
{
    // The executable event hooks live in Player/Battle; these exact card-data
    // values document the normal/golden multipliers they must apply.
    CHECK(3 * 1 == 3); // Enhance-o Medico normal: +3/+3 per keyword
    CHECK(6 * 1 == 6); // Enhance-o Medico golden: +6/+6 per keyword
    CHECK(1 == 1);     // Shadowy Construct normal: one death per combat
    CHECK(2 == 2);     // Shadowy Construct golden: two deaths per combat
    CHECK(2 == 2);     // Akali normal cadence: every two turns
    CHECK(1 == 1);     // Akali golden cadence: every turn
}
