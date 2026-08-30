#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - Patch 36.4 batch")
{
    CHECK(MenagerieTablewareRepeatCount(0) == 1);
    CHECK(MenagerieTablewareRepeatCount(1) == 2);
    CHECK(MenagerieTablewareRepeatCount(3) == 4);

    const auto menagerie = FindTavernSpellBehavior("BG34_272");
    CHECK(menagerie.effect == TavernSpellEffect::MENAGERIE_STATS);
    CHECK(menagerie.attack == 3);
    CHECK(menagerie.health == 3);

    const auto wave = FindTavernSpellBehavior("BG34_990");
    CHECK(wave.effect == TavernSpellEffect::ALL_STATS_AND_GOLDEN);
    CHECK(wave.attack == 3);
    CHECK(wave.health == 2);

    const auto apples = FindTavernSpellBehavior("BG28_966");
    CHECK(apples.effect == TavernSpellEffect::SHOP_STATS);
    CHECK(apples.attack == 1);
    CHECK(apples.health == 2);
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - targeted stat batch")
{
    const auto vision = FindTavernSpellBehavior("BG28_838");
    CHECK(vision.effect == TavernSpellEffect::SET_TARGET_STATS);
    CHECK(vision.attack == 20);
    CHECK(vision.health == 20);
    CHECK(TavernSpellRequiresTarget(vision.effect));

    const auto banana = FindTavernSpellBehavior("BG28_897");
    CHECK(banana.effect == TavernSpellEffect::TARGET_STATS);
    CHECK(banana.attack == 2);
    CHECK(banana.health == 2);
    CHECK(TavernSpellRequiresTarget(banana.effect));

    const auto tide = FindTavernSpellBehavior("BG32_815");
    CHECK(tide.effect == TavernSpellEffect::TARGET_STATS_REPEAT);
    CHECK(tide.race == Race::NAGA);
    CHECK(TavernSpellRequiresTarget(tide.effect));

    const auto deepwater = FindTavernSpellBehavior("BG35_149");
    CHECK(deepwater.effect == TavernSpellEffect::TARGET_AND_RACE);
    CHECK(deepwater.race == Race::MURLOC);
    CHECK(TavernSpellRequiresTarget(deepwater.effect));

    const auto repair = FindTavernSpellBehavior("BG36_624");
    CHECK(repair.effect == TavernSpellEffect::TARGET_STATS);
    CHECK(repair.attack == 4);
    CHECK(repair.health == 8);
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - one per type batch")
{
    const auto tea = FindTavernSpellBehavior("BG28_888");
    CHECK(tea.effect == TavernSpellEffect::ONE_PER_RACE_STATS);
    CHECK(tea.attack == 4);
    CHECK(tea.health == 4);
    CHECK(!TavernSpellRequiresTarget(tea.effect));
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - seeded random batch")
{
    for (const auto* id : {"BG33_811", "BG33_812", "BG35_951"})
    {
        const auto behavior = FindTavernSpellBehavior(id);
        CHECK(behavior.effect == TavernSpellEffect::RANDOM_STATS);
        CHECK(behavior.randomCount == 4);
        CHECK(behavior.gold == 0);
    }
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - unsupported is fail closed")
{
    const auto behavior = FindTavernSpellBehavior("BG28_503");
    CHECK(behavior.effect == TavernSpellEffect::NONE);
    CHECK(behavior.gold < 0);
}
