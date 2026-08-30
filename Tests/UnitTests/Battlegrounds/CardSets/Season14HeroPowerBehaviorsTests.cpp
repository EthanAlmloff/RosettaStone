// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviors] - batch has exact unique IDs")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS.size() == 8);

    for (std::size_t i = 0; i < SEASON14_HERO_POWER_BEHAVIORS.size(); ++i)
    {
        const auto& entry = SEASON14_HERO_POWER_BEHAVIORS[i];
        REQUIRE(FindSeason14HeroPowerBehavior(entry.id) != nullptr);
        REQUIRE(FindSeason14HeroPowerBehavior(entry.dbfID) != nullptr);
        CHECK(FindSeason14HeroPowerBehavior(entry.id)->dbfID == entry.dbfID);
        for (std::size_t j = i + 1; j < SEASON14_HERO_POWER_BEHAVIORS.size();
             ++j)
        {
            CHECK(entry.id != SEASON14_HERO_POWER_BEHAVIORS[j].id);
            CHECK(entry.dbfID != SEASON14_HERO_POWER_BEHAVIORS[j].dbfID);
        }
    }
}

TEST_CASE("[Season14HeroPowerBehaviors] - passive families are explicit")
{
    const auto* patched = FindSeason14HeroPowerBehavior(59399);
    REQUIRE(patched != nullptr);
    CHECK(patched->kind == Season14HeroPowerKind::STARTING_HEALTH);
    CHECK(patched->passive);

    const auto* manastorm = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_054");
    REQUIRE(manastorm != nullptr);
    CHECK(manastorm->kind == Season14HeroPowerKind::ECONOMY_COST_AURA);
    CHECK(manastorm->passive);

    const auto* lighting = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_085t");
    REQUIRE(lighting != nullptr);
    CHECK(lighting->kind == Season14HeroPowerKind::TAVERN_SPELL_AURA);
    CHECK(lighting->passive);

    const auto patchedModifiers = Season14HeroPowerBatch1Modifiers(59399);
    CHECK(patchedModifiers.StartingHealth(40) == 70);
    CHECK(patchedModifiers.StartingHealth(60) == 90);

    const auto manastormModifiers = Season14HeroPowerBatch1Modifiers(60405);
    CHECK(manastormModifiers.UpgradeCost(5) == 6);

    // Tavern Lighting buffs spell results; it is not a payment discount.
    CHECK(Season14HeroPowerBatch1Modifiers(122960).TavernSpellCost(3) == 3);
}

TEST_CASE("[Season14HeroPowerBehaviors] - no-target active effects are deterministic")
{
    Season14HeroPowerActivation activation{};

    CHECK(ResolveSeason14HeroPowerActivation(62269, 1, activation));
    CHECK(activation.goldDelta == 2);
    CHECK(activation.maxGoldDelta == 0);

    CHECK(ResolveSeason14HeroPowerActivation(62269, 4, activation));
    CHECK(activation.goldDelta == 5);

    CHECK(ResolveSeason14HeroPowerActivation(116921, 1, activation));
    CHECK(activation.goldDelta == 0);
    CHECK(activation.maxGoldDelta == 1);

    CHECK(!ResolveSeason14HeroPowerActivation(59399, 1, activation));
    CHECK(!ResolveSeason14HeroPowerActivation(0, 1, activation));
}
