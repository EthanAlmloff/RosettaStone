#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellOnAdjacentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnDestroyAdjacentCopyTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/WindfallTornadoDiscoverTask.hpp>
#include <map>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch72] adjacent-copy and adjacent-spell families preserve golden scaling") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);

  const auto& normalCopy = cards.at("BG28_308").power.GetTrigger()->GetTasks();
  const auto& goldenCopy = cards.at("BG28_308_G").power.GetTrigger()->GetTasks();
  CHECK(std::get<SimpleTasks::EndTurnDestroyAdjacentCopyTask>(normalCopy.front()).Both() == false);
  CHECK(std::get<SimpleTasks::EndTurnDestroyAdjacentCopyTask>(goldenCopy.front()).Both() == true);

  const auto& normalSpell = cards.at("BG34_920").power.GetDeathrattleTask();
  const auto& goldenSpell = cards.at("BG34_920_G").power.GetDeathrattleTask();
  CHECK(std::get<SimpleTasks::CastTavernSpellOnAdjacentTask>(normalSpell.front()).Both() == false);
  CHECK(std::get<SimpleTasks::CastTavernSpellOnAdjacentTask>(goldenSpell.front()).Both() == true);
}

TEST_CASE("[Batch72] Windfall Tornado sells into one or two Elemental Discovers") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);
  const auto& normal = cards.at("BG31_817").power.GetTrigger()->GetTasks();
  const auto& golden = cards.at("BG31_817_G").power.GetTrigger()->GetTasks();
  CHECK(std::get<SimpleTasks::WindfallTornadoDiscoverTask>(normal.front()).Choices() == 1);
  CHECK(std::get<SimpleTasks::WindfallTornadoDiscoverTask>(golden.front()).Choices() == 2);
}

TEST_CASE("[Batch72] golden Macaw triggers the leftmost deathrattle twice") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);
  const auto& tasks = cards.at("TB_BaconUps_135").power.GetRallyTask();
  REQUIRE(tasks.size() == 1);
  REQUIRE(std::holds_alternative<SimpleTasks::TriggerLeftmostDeathrattleTask>(tasks[0]));
  CHECK(std::get<SimpleTasks::TriggerLeftmostDeathrattleTask>(tasks[0]).Repeats() == 2);
}
