#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>

#include <doctest/doctest.h>

#include <array>

using namespace RosettaStone;
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - initial lookup table batch")
{
    struct Expected
    {
        const char* id;
        TavernSpellEffect effect;
        int attack;
        int health;
        int gold;
        Race race;
    };

    constexpr std::array expected{
        Expected{ "BG28_168", TavernSpellEffect::ALL_STATS, 1, 1, 0,
                  Race::INVALID },
        Expected{ "BG28_169", TavernSpellEffect::ALL_STATS, 4, 4, 0,
                  Race::INVALID },
        Expected{ "BG33_813", TavernSpellEffect::LEFTMOST_STATS, 6, 6, 0,
                  Race::INVALID },
        Expected{ "BG33_817", TavernSpellEffect::DIVINE_SHIELD_ATTACK, 6, 0,
                  0, Race::INVALID },
        Expected{ "BG35_922", TavernSpellEffect::ALL_AND_RACE, 2, 2, 0,
                  Race::NAGA },
        Expected{ "BG36_246",
                  TavernSpellEffect::ALL_RACE_AND_DIVINE_SHIELD,
                  2,
                  1,
                  0,
                  Race::DRAGON },
        Expected{ "BG28_810", TavernSpellEffect::NONE, 0, 0, 1,
                  Race::INVALID },
        Expected{ "BG33_815", TavernSpellEffect::NONE, 0, 0, 2,
                  Race::INVALID },
    };

    for (const auto& item : expected)
    {
        const auto behavior = FindTavernSpellBehavior(item.id);
        CHECK(behavior.effect == item.effect);
        CHECK(behavior.attack == item.attack);
        CHECK(behavior.health == item.health);
        CHECK(behavior.gold == item.gold);
        CHECK(behavior.race == item.race);
    }
}

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
    CHECK(tide.race == RosettaStone::Race::NAGA);
    CHECK(TavernSpellRequiresTarget(tide.effect));

    const auto deepwater = FindTavernSpellBehavior("BG35_149");
    CHECK(deepwater.effect == TavernSpellEffect::TARGET_AND_RACE);
    CHECK(deepwater.race == RosettaStone::Race::MURLOC);
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
    const auto behavior = FindTavernSpellBehavior("BG36_UNSUPPORTED");
    CHECK(behavior.effect == TavernSpellEffect::NONE);
    CHECK(behavior.gold < 0);
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - simple keyword/economy batch")
{
    struct Expected
    {
        const char* id;
        TavernSpellEffect effect;
        int attack;
        int health;
        int value;
        int gold;
        bool target;
    };

    constexpr std::array expected{
        Expected{ "BG28_503", TavernSpellEffect::TARGET_STATS_AND_TAUNT, 0,
                  3, 0, 0, true },
        Expected{ "BG28_507", TavernSpellEffect::TARGET_DIVINE_SHIELD, 0, 0,
                  0, 0, true },
        Expected{ "BG28_520", TavernSpellEffect::TARGET_STATS_TOGGLE_TAUNT, 1,
                  2, 0, 0, true },
        Expected{ "BG28_825", TavernSpellEffect::TARGET_STATS_AND_TAUNT, 7,
                  7, 0, 0, true },
        Expected{ "BG28_500", TavernSpellEffect::SET_PLAYER_ARMOR, 0, 0, 5,
                  0, false },
        Expected{ "BG28_800", TavernSpellEffect::NEXT_TURN_GOLD, 0, 0, 2,
                  0, false },
        Expected{ "BG28_805", TavernSpellEffect::INCREASE_MAX_GOLD, 0, 0, 1,
                  0, false },
        Expected{ "BG28_827", TavernSpellEffect::FREE_REFRESHES, 0, 0, 2,
                  0, false },
        Expected{ "BG28_886", TavernSpellEffect::SHOP_STATS_PERSISTENT, 2, 2,
                  0, 0, false },
        Expected{ "BG28_571", TavernSpellEffect::SPELL_COSTS_HEALTH, 0, 0, 0,
                  1, false },
    };

    for (const auto& item : expected)
    {
        const auto behavior = FindTavernSpellBehavior(item.id);
        CHECK(behavior.effect == item.effect);
        CHECK(behavior.attack == item.attack);
        CHECK(behavior.health == item.health);
        CHECK(behavior.value == item.value);
        CHECK(TavernSpellRequiresTarget(behavior.effect) == item.target);
        CHECK(behavior.gold == item.gold);
    }
}

TEST_CASE("[Battlegrounds : TavernSpellBehaviors] - generated and tribal batch")
{
    struct Expected
    {
        const char* id;
        TavernSpellEffect effect;
        int attack;
        int health;
        int randomCount;
        bool target;
    };

    constexpr std::array expected{
        Expected{ "BG28_845", TavernSpellEffect::TARGET_SHARED_RACE_STATS, 3,
                  3, 0, true },
        Expected{ "BG35_912",
                  TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT,
                  3,
                  3,
                  0,
                  true },
        Expected{ "EBG_Spell_017", TavernSpellEffect::TARGET_GOLDEN, 0, 0, 0,
                  true },
        Expected{ "BG28_830", TavernSpellEffect::RANDOM_SHOP_GOLDEN, 0, 0, 0,
                  false },
        Expected{ "BG28_504", TavernSpellEffect::RANDOM_MINION_TO_HAND,
                  0, 0, 0, false },
        Expected{ "BG33_814",
                  TavernSpellEffect::RANDOM_COMMON_RACE_MINION_TO_HAND,
                  0,
                  0,
                  0,
                  false },
        Expected{ "BG28_512", TavernSpellEffect::STEAL_RANDOM_SHOP_MINION, 0,
                  0, 0, false },
        Expected{ "BG34_444",
                  TavernSpellEffect::RANDOM_SHOP_STATS_ON_REFRESH,
                  8,
                  8,
                  0,
                  false },
        Expected{ "EBG_Spell_032",
                  TavernSpellEffect::SELL_TARGET_GIVE_RANDOM_STATS, 0, 0, 0,
                  true },
    };

    for (const auto& item : expected)
    {
        const auto behavior = FindTavernSpellBehavior(item.id);
        CHECK(behavior.effect == item.effect);
        CHECK(behavior.attack == item.attack);
        CHECK(behavior.health == item.health);
        CHECK(behavior.randomCount == item.randomCount);
        CHECK(TavernSpellRequiresTarget(behavior.effect) == item.target);
        CHECK(behavior.gold == 0);
    }

    CHECK(TavernSpellTargetIsLegal(TavernSpellEffect::TARGET_GOLDEN, 4,
                                   false));
    CHECK(!TavernSpellTargetIsLegal(TavernSpellEffect::TARGET_GOLDEN, 5,
                                    false));
    CHECK(!TavernSpellTargetIsLegal(TavernSpellEffect::TARGET_GOLDEN, 4,
                                    true));
}
