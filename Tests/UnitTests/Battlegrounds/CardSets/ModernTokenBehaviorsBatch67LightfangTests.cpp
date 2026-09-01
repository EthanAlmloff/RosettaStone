#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch67.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <map>
#include <utility>
#include <variant>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch67] - Lightfang end-turn family has exact stat payloads") {
  std::map<std::string, CardDef> cards;
  ModernTokenBehaviorsBatch67::AddAll(cards);
  REQUIRE(cards.size() == 6);
  for (const auto& [id, amount] : {std::pair{"BGS_009", 2}, std::pair{"TB_BaconUps_082", 4}}) {
    REQUIRE(cards.at(id).power.GetTrigger().has_value());
    const auto& tasks = cards.at(id).power.GetTrigger()->GetTasks();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::OnePerTypeRallyBuffTask>(tasks.front()));
    const auto& task = std::get<SimpleTasks::OnePerTypeRallyBuffTask>(tasks.front());
    CHECK(task.GetAttack() == amount);
    CHECK(task.GetHealth() == amount);
  }
}

TEST_CASE("[Batch67] - Lightfang buffs one friendly minion per race") {
  Player player;
  Minion beast{Cards::FindCardByID("BG21_001")};
  Minion demon{Cards::FindCardByID("BG21_004")};
  player.recruitField.Add(beast);
  player.recruitField.Add(demon);
  const int beastAttack = player.recruitField[0].GetAttack();
  const int beastHealth = player.recruitField[0].GetHealth();
  const int demonHealth = player.recruitField[1].GetHealth();
  CHECK(SimpleTasks::OnePerTypeRallyBuffTask{2, 2, 1}.Run(
      player, player.recruitField[0]) == TaskStatus::COMPLETE);
  CHECK(player.recruitField[0].GetAttack() == beastAttack + 2);
  CHECK(player.recruitField[0].GetHealth() == beastHealth + 2);
  CHECK(player.recruitField[1].GetHealth() == demonHealth + 2);
}
