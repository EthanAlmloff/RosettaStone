// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch12.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <map>
#include <string>
#include <variant>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 12 lobster")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch12::AddAll(cards);

    REQUIRE_EQ(cards.size(), 2);
    for (const auto* id : { "BG36_202", "BG36_202_G" })
    {
        CAPTURE(id);
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK_EQ(cards.at(id).power.GetDeathrattleTask().size(), 1);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 12 lobster scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch12::AddAll(cards);

    const auto* normal = std::get_if<SimpleTasks::RandomFriendlyRaceTask>(
        &cards.at("BG36_202").power.GetDeathrattleTask().front());
    const auto* golden = std::get_if<SimpleTasks::RandomFriendlyRaceTask>(
        &cards.at("BG36_202_G").power.GetDeathrattleTask().front());
    REQUIRE(normal != nullptr);
    REQUIRE(golden != nullptr);
    CHECK_EQ(normal->GetAttack(), 1);
    CHECK_EQ(normal->GetHealth(), 1);
    CHECK_EQ(normal->GetAmount(), 1);
    CHECK(normal->ImprovesFutureLobsters());
    CHECK_EQ(golden->GetAttack(), 2);
    CHECK_EQ(golden->GetHealth(), 2);
    CHECK_EQ(golden->GetAmount(), 1);
    CHECK(golden->ImprovesFutureLobsters());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 12 persistent lobster aura")
{
    Season14State state;
    CHECK_EQ(state.FutureLobsterStats(), std::pair{ 0, 0 });
    state.ImproveFutureLobsters(1, 1);
    state.ImproveFutureLobsters(2, 2);
    CHECK_EQ(state.FutureLobsterStats(), std::pair{ 3, 3 });
}
