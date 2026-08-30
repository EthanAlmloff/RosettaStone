// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch10.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 10 Lullabot")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch10::AddAll(cards);
    for (const auto* id : { "BG26_146", "BG26_146_G", "BG26_146e2",
                            "BG26_146_Ge2" })
    {
        CAPTURE(id);
        REQUIRE(cards.contains(id));
    }
    REQUIRE(cards.at("BG26_146").power.GetTrigger().has_value());
    REQUIRE(cards.at("BG26_146_G").power.GetTrigger().has_value());
    CHECK(cards.at("BG26_146").power.GetTrigger()->GetTriggerType() ==
          TriggerType::TURN_END);
    CHECK(cards.at("BG26_146_G").power.GetTrigger()->GetTriggerType() ==
          TriggerType::TURN_END);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 10 scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch10::AddAll(cards);
    REQUIRE(cards.at("BG26_146e2").power.GetEnchant().has_value());
    REQUIRE(cards.at("BG26_146_Ge2").power.GetEnchant().has_value());
}
