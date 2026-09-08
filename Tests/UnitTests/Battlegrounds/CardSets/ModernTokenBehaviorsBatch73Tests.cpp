#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AfterSellRaceStatsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BloodGemRaceBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <map>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch73] golden sell/stat families are registered with exact scaling") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);
  for (const auto* id : {"BG31_360", "BG31_360_G", "BG35_437", "BG35_437_G",
                         "BG34_Giant_072", "BG34_Giant_072_G"})
    CHECK(cards.contains(id));

  const auto& behemoth = cards.at("BG31_360").power.GetTrigger()->GetTasks();
  const auto& goldenBehemoth = cards.at("BG31_360_G").power.GetTrigger()->GetTasks();
  CHECK(std::get<SimpleTasks::AfterSellRaceStatsTask>(behemoth.front()).Multiplier() == 1);
  CHECK(std::get<SimpleTasks::AfterSellRaceStatsTask>(goldenBehemoth.front()).Multiplier() == 2);

  const auto& vinespeaker = cards.at("BG35_437").power.GetTrigger()->GetTasks();
  const auto& goldenVinespeaker = cards.at("BG35_437_G").power.GetTrigger()->GetTasks();
  const auto& normalBonus = std::get<SimpleTasks::BloodGemRaceBonusTask>(vinespeaker.front());
  const auto& goldenBonus = std::get<SimpleTasks::BloodGemRaceBonusTask>(goldenVinespeaker.front());
  CHECK(normalBonus.GetRace() == Race::ALL);
  CHECK(normalBonus.GetAttack() == 2);
  CHECK(normalBonus.GetHealth() == 0);
  CHECK(goldenBonus.GetAttack() == 4);
  CHECK(goldenBonus.GetHealth() == 0);
}

TEST_CASE("[Batch73] Timewarped Skipper gives one or two tier-one minions to hand") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);
  const auto& normal = cards.at("BG34_Giant_072").power.GetTrigger()->GetTasks();
  const auto& golden = cards.at("BG34_Giant_072_G").power.GetTrigger()->GetTasks();
  REQUIRE(normal.size() == 1);
  CHECK(std::holds_alternative<SimpleTasks::RandomCardToHandTask>(normal.front()));
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normal.front()).GetTier() == 1);
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(normal.front()).GetAmount() == 1);
  REQUIRE(golden.size() == 1);
  CHECK(std::holds_alternative<SimpleTasks::RandomCardToHandTask>(golden.front()));
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(golden.front()).GetTier() == 1);
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(golden.front()).GetAmount() == 2);
}
