#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ActivateBehaviors.hpp>

#include <map>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : Activate] - simple target families are registered")
{
    std::map<std::string, CardDef> cards;
    ActivateBehaviors::AddAll(cards);

    REQUIRE_EQ(cards.size(), 4);
    CHECK(cards.contains("BG36_345"));
    CHECK(cards.contains("BG36_345_G"));
    CHECK(cards.contains("BG36_356"));
    CHECK(cards.contains("BG36_356_G"));

    CHECK_EQ(cards.at("BG36_345").power.GetActivate()->cost, 1);
    CHECK_EQ(cards.at("BG36_345_G").power.GetActivate()->attack, 6);
    CHECK_EQ(cards.at("BG36_356").power.GetActivate()->health, 50);
    CHECK_EQ(cards.at("BG36_356_G").power.GetActivate()->health, 100);
}

TEST_CASE("[Battlegrounds : Activate] - Activate is separate from Battlecry")
{
    std::map<std::string, CardDef> cards;
    ActivateBehaviors::AddAll(cards);
    for (const auto& [id, definition] : cards)
    {
        CAPTURE(id);
        CHECK(definition.power.GetBattlecryTask().empty());
        CHECK(definition.power.GetRallyTask().empty());
        CHECK(definition.power.GetActivate().has_value());
    }
}
