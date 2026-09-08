#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmFodderRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRandomRaceKeywordTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <map>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch71] token and golden registrations are complete") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);
  for (const auto* id : {
           "BG19_010", "BG19_010_G", "BG19_010t", "BG19_010_Gt",
           "BG22_202", "BG22_202_G", "BG25_009_G", "BG25_010_G",
           "BG29_140", "BG29_140_G", "BG35_151", "BG35_151_G",
           "BG36_730_G", "TB_BaconUps_082", "TB_BaconUps_141",
           "TB_BaconUps_165", "TB_BaconUps_201", "BG25_010t",
           "BG26_888", "BG26_888_G", "BG31_HERO_801pt",
           "BG31_HERO_801pt_G", "BGS_115t", "BGS_115t_G", "BG_BOT_312t",
           "BG_ICC_026t", "BG_ICC_026t_G", "TB_BaconUps_032t",
           "BG24_004_G", "BG26_175_G"}) {
    CHECK(cards.contains(id));
  }
}

TEST_CASE("[Batch71] golden scaling is preserved in declarative task payloads") {
  std::map<std::string, CardDef> cards;
  GeneratedBehaviorMappings::AddAll(cards);

  const auto& sellNormal = cards.at("BG22_202").power.GetTrigger()->GetTasks();
  const auto& sellGolden = cards.at("BG22_202_G").power.GetTrigger()->GetTasks();
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(sellNormal.front()).GetAmount() == 1);
  CHECK(std::get<SimpleTasks::RandomCardToHandTask>(sellGolden.front()).GetAmount() == 2);

  const auto& fodderNormal = cards.at("BG35_151").power.GetTrigger()->GetTasks();
  const auto& fodderGolden = cards.at("BG35_151_G").power.GetTrigger()->GetTasks();
  CHECK(std::get<SimpleTasks::ArmFodderRefreshTask>(fodderNormal.front()).Amount() == 1);
  CHECK(std::get<SimpleTasks::ArmFodderRefreshTask>(fodderGolden.front()).Amount() == 2);
  CHECK(std::get<SimpleTasks::ArmFodderRefreshTask>(fodderNormal.front()).Refreshes() == 3);

  const auto& types = cards.at("TB_BaconUps_082").power.GetTrigger()->GetTasks();
  const auto& typeBuff = std::get<SimpleTasks::OnePerTypeRallyBuffTask>(types.front());
  CHECK(typeBuff.GetAttack() == 8);
  CHECK(typeBuff.GetHealth() == 8);

  const auto& venomNormal = cards.at("BG26_888").power.GetDeathrattleTask();
  const auto& venomGolden = cards.at("BG26_888_G").power.GetDeathrattleTask();
  CHECK(std::holds_alternative<SimpleTasks::RallyRandomRaceKeywordTask>(venomNormal.front()));
  CHECK(std::holds_alternative<SimpleTasks::RallyRandomRaceKeywordTask>(venomGolden.front()));
}
