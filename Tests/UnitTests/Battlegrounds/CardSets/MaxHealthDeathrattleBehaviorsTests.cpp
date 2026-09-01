#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MaxHealthDeathrattleTask.hpp>

#include <map>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds] - Impulsive Trickster max-health deathrattle family")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);

    for (const auto* id : {"BG21_006", "BG21_006_G"})
        REQUIRE(cards.contains(id));

    const auto& normal = cards.at("BG21_006").power.GetDeathrattleTask();
    const auto& golden = cards.at("BG21_006_G").power.GetDeathrattleTask();
    REQUIRE(normal.size() == 1);
    REQUIRE(golden.size() == 1);
    CHECK(std::holds_alternative<SimpleTasks::MaxHealthDeathrattleTask>(normal.front()));
    CHECK(std::holds_alternative<SimpleTasks::MaxHealthDeathrattleTask>(golden.front()));
    CHECK(std::get<SimpleTasks::MaxHealthDeathrattleTask>(normal.front()).Repeats() == 1);
    CHECK(std::get<SimpleTasks::MaxHealthDeathrattleTask>(golden.front()).Repeats() == 2);
}

TEST_CASE("[Battlegrounds] - max-health transfer survives source damage and excludes source")
{
    Player player;
    player.isInCombat = true;
    Minion target{Cards::FindCardByID("BG21_001")};
    const int targetHealth = target.GetHealth();
    player.battleField.Add(target);

    Minion source{Cards::FindCardByID("BG21_006")};
    source.SetHealth(source.GetHealth() + 5);
    source.TakeDamage(4);
    REQUIRE(source.GetHealth() == 4);
    CHECK(source.GetMaxHealth() == 8);

    SimpleTasks::MaxHealthDeathrattleTask task;
    CHECK(task.Run(player, source) == TaskStatus::COMPLETE);
    CHECK(player.battleField[0].GetHealth() == targetHealth + source.GetMaxHealth());
}
