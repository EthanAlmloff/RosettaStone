#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatSpellScaledRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthHandMurlocSummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/SpellcraftMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>

#include <map>
#include <string>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : EventCounterBehaviors] - Photobomber damage paths")
{
    std::map<std::string, CardDef> cards;
    EventCounterBehaviors::AddAll(cards);

    REQUIRE(cards.contains("BG34_780"));
    REQUIRE(cards.contains("BG34_780_G"));
    const auto& normal = cards.at("BG34_780").power.GetDeathrattleTask();
    const auto& golden = cards.at("BG34_780_G").power.GetDeathrattleTask();
    REQUIRE(normal.size() == 1);
    REQUIRE(golden.size() == 1);
    const auto& n = std::get<SimpleTasks::HighestHealthEnemyDamageTask>(normal.front());
    const auto& g = std::get<SimpleTasks::HighestHealthEnemyDamageTask>(golden.front());
    CHECK(n.BaseDamage() == 2);
    CHECK(n.Repeats() == 1);
    CHECK(n.DamagePerSpell() == 1);
    CHECK(g.BaseDamage() == 2);
    CHECK(g.Repeats() == 2);
    CHECK(g.DamagePerSpell() == 1);
}

TEST_CASE("[Battlegrounds : EventCounterBehaviors] - Fire-forged Evoker scales")
{
    std::map<std::string, CardDef> cards;
    EventCounterBehaviors::AddAll(cards);
    REQUIRE(cards.contains("BG32_822"));
    REQUIRE(cards.contains("BG32_822_G"));
    const auto& normal = cards.at("BG32_822").power.GetStartCombatTask().front();
    const auto& golden = cards.at("BG32_822_G").power.GetStartCombatTask().front();
    const auto& n = std::get<SimpleTasks::StartCombatSpellScaledRaceBuffTask>(normal);
    const auto& g = std::get<SimpleTasks::StartCombatSpellScaledRaceBuffTask>(golden);
    CHECK(n.Attack() == 2);
    CHECK(n.Health() == 1);
    CHECK(n.ImprovementPerSpell() == 1);
    CHECK(g.Attack() == 4);
    CHECK(g.Health() == 2);
    CHECK(g.ImprovementPerSpell() == 1);
}

TEST_CASE("[Battlegrounds : SpellcraftMinionBehaviors] - Tranquil Meditative registered")
{
    std::map<std::string, CardDef> cards;
    SpellcraftMinionBehaviors::AddAll(cards);
    CHECK(cards.contains("BG32_835"));
    CHECK(cards.contains("BG32_835_G"));
}

TEST_CASE("[Battlegrounds : SpellcraftMinionBehaviors] - Tavern spell trigger pair registered")
{
    std::map<std::string, CardDef> cards;
    SpellcraftMinionBehaviors::AddAll(cards);
    CHECK(cards.contains("BG28_551"));
    CHECK(cards.contains("BG28_551_G"));
    CHECK(cards.contains("BG28_741"));
    CHECK(cards.contains("BG28_741_G"));
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - Highest-health Murloc deathrattle pair")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);
    REQUIRE(cards.contains("BG26_350"));
    REQUIRE(cards.contains("BG26_350_G"));
    const auto& normal = cards.at("BG26_350").power.GetDeathrattleTask();
    const auto& golden = cards.at("BG26_350_G").power.GetDeathrattleTask();
    REQUIRE(normal.size() == 1);
    REQUIRE(golden.size() == 1);
    CHECK(std::get<SimpleTasks::HighestHealthHandMurlocSummonTask>(normal.front()).Count() == 1);
    CHECK(std::get<SimpleTasks::HighestHealthHandMurlocSummonTask>(golden.front()).Count() == 2);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - Tier goldenize battlecry pair")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);
    REQUIRE(cards.contains("BG25_034"));
    REQUIRE(cards.contains("BG25_034_G"));
    CHECK(std::get<SimpleTasks::GoldenizeTierMinionTask>(
              cards.at("BG25_034").power.GetBattlecryTask().front()).Count() == 1);
    CHECK(std::get<SimpleTasks::GoldenizeTierMinionTask>(
              cards.at("BG25_034_G").power.GetBattlecryTask().front()).Count() == 2);
}
