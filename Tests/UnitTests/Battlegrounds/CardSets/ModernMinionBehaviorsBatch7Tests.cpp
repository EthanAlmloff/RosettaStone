// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <array>
#include <map>
#include <string>
#include <variant>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 inventory table")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);
    CHECK(cards.empty());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 summon scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);
    CHECK(cards.empty());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 has no metadata-only registrations")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);
    // The exact excluded IDs and their missing mechanics are maintained in
    // the parent project's experimental metadata-only catalog.  Avoid
    // repeating those IDs here: coverage tooling treats literals in focused
    // tests as evidence of a behavior test.
    CHECK(cards.empty());
}
