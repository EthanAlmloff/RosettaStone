// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeftmostFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <array>
#include <map>
#include <string>
#include <variant>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 9 inventory")
{
    constexpr std::array ids{
        "BG26_805",   "BG26_805_G",  "BG26_963",   "BG26_963_G",
        "BG24_500",   "BG24_500_G",  "BG29_810",   "BG29_810_G",
        "BG36_620",   "BG36_620_G",  "BG26_805e",  "BG26_963e",
    };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch9::AddAll(cards);
    for (const auto* id : ids)
    {
        CAPTURE(id);
        REQUIRE(cards.contains(id));
        CHECK_FALSE(Cards::FindCardByID(id).id.empty());
    }
    CHECK_EQ(cards.size(), ids.size());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 9 race scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch9::AddAll(cards);

    REQUIRE_EQ(cards.at("BG26_805").power.GetStartCombatTask().size(), 1);
    REQUIRE_EQ(cards.at("BG26_805_G").power.GetStartCombatTask().size(), 2);
    for (const auto* id : { "BG26_805", "BG26_805_G" })
    {
        CAPTURE(id);
        CHECK(std::holds_alternative<FriendlyRaceEnchantmentTask>(
            cards.at(id).power.GetStartCombatTask().front()));
    }

    REQUIRE_EQ(cards.at("BG26_963").power.GetBattlecryTask().size(), 1);
    REQUIRE_EQ(cards.at("BG26_963").power.GetStartCombatTask().size(), 1);
    REQUIRE_EQ(cards.at("BG26_963_G").power.GetBattlecryTask().size(), 2);
    REQUIRE_EQ(cards.at("BG26_963_G").power.GetStartCombatTask().size(), 2);
    CHECK(std::holds_alternative<FriendlyRaceEnchantmentTask>(
        cards.at("BG26_963_G").power.GetBattlecryTask().back()));

    REQUIRE(cards.at("BG26_805e").power.GetEnchant().has_value());
    REQUIRE(cards.at("BG26_963e").power.GetEnchant().has_value());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 9 target and damage scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch9::AddAll(cards);

    REQUIRE_EQ(cards.at("BG24_500").power.GetStartCombatTask().size(), 1);
    REQUIRE_EQ(cards.at("BG24_500_G").power.GetStartCombatTask().size(), 1);
    CHECK(std::holds_alternative<RandomFriendlyRaceTask>(
        cards.at("BG24_500").power.GetStartCombatTask().front()));
    CHECK(std::holds_alternative<RandomFriendlyRaceTask>(
        cards.at("BG24_500_G").power.GetStartCombatTask().front()));

    CHECK(std::holds_alternative<LeftmostFriendlyRaceTask>(
        cards.at("BG29_810").power.GetStartCombatTask().front()));
    CHECK(std::holds_alternative<LeftmostFriendlyRaceTask>(
        cards.at("BG29_810_G").power.GetStartCombatTask().front()));

    REQUIRE_EQ(cards.at("BG36_620").power.GetStartCombatTask().size(), 1);
    REQUIRE_EQ(cards.at("BG36_620_G").power.GetStartCombatTask().size(), 2);
    CHECK(std::holds_alternative<DamageTask>(
        cards.at("BG36_620_G").power.GetStartCombatTask().front()));
}

