#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch33.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch33] - fixed friendly-race Battlecry payloads") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch33::AddAll(cards);
  struct Row { const char* id; Race race; int attack; int health; };
  for (const auto& row : {Row{"BGS_001", Race::DEMON, 2, 2}, Row{"TB_BaconUps_062", Race::DEMON, 4, 4},
                          Row{"BGS_038", Race::DRAGON, 2, 2}, Row{"TB_BaconUps_108", Race::DRAGON, 4, 4}}) {
    REQUIRE(cards.contains(row.id)); REQUIRE(cards.at(row.id).power.GetBattlecryTask().size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::RandomFriendlyRaceTask>(cards.at(row.id).power.GetBattlecryTask().front()));
    const auto& task = std::get<SimpleTasks::RandomFriendlyRaceTask>(cards.at(row.id).power.GetBattlecryTask().front());
    CHECK(task.GetAttack() == row.attack); CHECK(task.GetHealth() == row.health);
  }
}
