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
    constexpr std::array ids{ "BG25_009", "BG25_009_G" };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);

    for (const auto* id : ids)
    {
        REQUIRE(cards.contains(id));
        const auto metadata = Cards::FindCardByID(id);
        REQUIRE_FALSE(metadata.id.empty());
        CHECK_EQ(metadata.id, id);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 summon scaling")
{
    struct ExpectedSummon
    {
        const char* id;
        const char* tokenID;
        int amount;
    };

    constexpr std::array expected{
        ExpectedSummon{ "BG25_009", "BG25_008", 1 },
        ExpectedSummon{ "BG25_009_G", "BG25_008_G", 1 },
    };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);

    for (const auto& row : expected)
    {
        CAPTURE(row.id);
        REQUIRE(cards.contains(row.id));
        const auto& deathrattles = cards.at(row.id).power.GetDeathrattleTask();
        REQUIRE_EQ(deathrattles.size(), 1);
        const auto* summon = std::get_if<SimpleTasks::SummonTask>(&deathrattles.front());
        REQUIRE(summon != nullptr);
        CHECK_EQ(summon->m_cardID, row.tokenID);
        CHECK_EQ(summon->m_amount, row.amount);
        CHECK(cards.at(row.id).power.GetBattlecryTask().empty());
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 has no metadata-only registrations")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);
    // The exact excluded IDs and their missing mechanics are maintained in
    // the parent project's experimental metadata-only catalog.  Avoid
    // repeating those IDs here: coverage tooling treats literals in focused
    // tests as evidence of a behavior test.
    CHECK_EQ(cards.size(), 2);
}
