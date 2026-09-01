#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/BattlegroundsCardsGen.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageHeroTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds] - Wrath Weaver golden trigger doubles its buff") {
  std::map<std::string, CardDef> cards;
  BattlegroundsCardsGen::AddAll(cards);
  REQUIRE(cards.contains("BGS_004"));
  REQUIRE(cards.contains("TB_BaconUps_079"));
  const auto& trigger = cards.at("TB_BaconUps_079").power.GetTrigger();
  REQUIRE(trigger.has_value());
  const auto& tasks = trigger->GetTasks();
  REQUIRE(tasks.size() == 3);
  REQUIRE(std::holds_alternative<SimpleTasks::DamageHeroTask>(tasks[0]));
  REQUIRE(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(tasks[1]));
  REQUIRE(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(tasks[2]));
  CHECK(std::get<SimpleTasks::AddEnchantmentTask>(tasks[1]).CardID() == "BGS_004e");
  CHECK(std::get<SimpleTasks::AddEnchantmentTask>(tasks[2]).CardID() == "BGS_004e");
}
