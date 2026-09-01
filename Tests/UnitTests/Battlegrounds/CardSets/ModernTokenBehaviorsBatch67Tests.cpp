#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeTavernForFriendlyDemonsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackGainHealthTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch67] - Famished Felbat uses one consume per friendly Demon") {
  std::map<std::string, CardDef> cards;
  ModernTokenBehaviorsBatch67::AddAll(cards);
  REQUIRE(cards.size() >= 2);
  for (const auto* id : {"BG21_005", "BG21_005_G"}) {
    REQUIRE(cards.at(id).power.GetTrigger().has_value());
    const auto& tasks = cards.at(id).power.GetTrigger()->GetTasks();
    REQUIRE(tasks.size() == 1);
    CHECK(std::holds_alternative<SimpleTasks::ConsumeTavernForFriendlyDemonsTask>(tasks.front()));
  }
  CHECK(std::get<SimpleTasks::ConsumeTavernForFriendlyDemonsTask>(
      cards.at("BG21_005_G").power.GetTrigger()->GetTasks().front()).Multiplier() == 2);
}

TEST_CASE("[Batch67] - Whelp listens only to positive persistent Dragon attack gains") {
  std::map<std::string, CardDef> cards;
  ModernTokenBehaviorsBatch67::AddAll(cards);
  REQUIRE(cards.at("BG21_013").power.GetTrigger().has_value());
  REQUIRE(cards.at("BG21_013_G").power.GetTrigger().has_value());
  const auto& normal = cards.at("BG21_013").power.GetTrigger()->GetTasks();
  const auto& golden = cards.at("BG21_013_G").power.GetTrigger()->GetTasks();
  REQUIRE(std::holds_alternative<SimpleTasks::AttackGainHealthTask>(normal.front()));
  CHECK(std::get<SimpleTasks::AttackGainHealthTask>(normal.front()).GetHealth() == 1);
  CHECK(std::get<SimpleTasks::AttackGainHealthTask>(golden.front()).GetHealth() == 2);
}

TEST_CASE("[Batch67] - Famished Felbat consumes for every Demon") {
  Player player;
  player.returnMinionCallback = [](int) {};
  player.recruitField.Add(Minion{Cards::FindCardByID("BG21_005")});
  player.recruitField.Add(Minion{Cards::FindCardByID("BG21_004")});
  player.tavern.fieldZone.Add(Minion{Cards::FindCardByID("BG21_001")});
  player.tavern.fieldZone.Add(Minion{Cards::FindCardByID("BG21_002")});
  const int first = player.recruitField[0].GetHealth();
  const int second = player.recruitField[1].GetHealth();
  CHECK(SimpleTasks::ConsumeTavernForFriendlyDemonsTask{1}.Run(
      player, player.recruitField[0]) == TaskStatus::COMPLETE);
  CHECK(player.tavern.fieldZone.GetCount() == 0);
  CHECK(player.recruitField[0].GetHealth() > first);
  CHECK(player.recruitField[1].GetHealth() > second);
}
