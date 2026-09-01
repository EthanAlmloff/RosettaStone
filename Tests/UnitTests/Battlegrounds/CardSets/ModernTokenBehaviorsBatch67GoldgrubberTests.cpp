#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnGoldenCountSelfBuffTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch67] - Goldgrubber end-turn family has normal/golden amounts") {
  std::map<std::string, CardDef> cards;
  ModernTokenBehaviorsBatch67::AddAll(cards);
  REQUIRE(cards.size() == 4);
  for (const auto& [id, amount] : {std::pair{"BGS_066", 2}, std::pair{"TB_BaconUps_130", 4}}) {
    REQUIRE(cards.at(id).power.GetTrigger().has_value());
    const auto& tasks = cards.at(id).power.GetTrigger()->GetTasks();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::EndTurnGoldenCountSelfBuffTask>(tasks.front()));
    CHECK(std::get<SimpleTasks::EndTurnGoldenCountSelfBuffTask>(tasks.front()).Amount() == amount);
  }
}

TEST_CASE("[Batch67] - Goldgrubber counts friendly Golden minions") {
  Player player;
  Minion source{Cards::FindCardByID("BGS_066")};
  Minion golden{Cards::FindCardByID("TB_BaconUps_130")};
  REQUIRE(golden.IsGolden());
  player.recruitField.Add(source);
  player.recruitField.Add(golden);
  const int attack = player.recruitField[0].GetAttack();
  const int health = player.recruitField[0].GetHealth();
  CHECK(SimpleTasks::EndTurnGoldenCountSelfBuffTask{2}.Run(
      player, player.recruitField[0]) == TaskStatus::COMPLETE);
  CHECK(player.recruitField[0].GetAttack() == attack + 2);
  CHECK(player.recruitField[0].GetHealth() == health + 2);
}
