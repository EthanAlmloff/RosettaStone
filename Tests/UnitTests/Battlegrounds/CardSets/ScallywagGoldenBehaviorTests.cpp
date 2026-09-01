#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/BattlegroundsCardsGen.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds] - Scallywag golden deathrattle summons and attacks golden token") {
  std::map<std::string, CardDef> cards;
  BattlegroundsCardsGen::AddAll(cards);
  REQUIRE(cards.contains("BGS_061"));
  REQUIRE(cards.contains("TB_BaconUps_141"));
  REQUIRE(cards.contains("BGS_061t"));
  REQUIRE(cards.contains("TB_BaconUps_141t"));
  const auto& tasks = cards.at("TB_BaconUps_141").power.GetDeathrattleTask();
  REQUIRE(tasks.size() == 2);
  REQUIRE(std::holds_alternative<SimpleTasks::SummonTask>(tasks[0]));
  REQUIRE(std::holds_alternative<SimpleTasks::AttackTask>(tasks[1]));
  const auto& summon = std::get<SimpleTasks::SummonTask>(tasks[0]);
  CHECK(summon.m_cardID == "TB_BaconUps_141t");
  CHECK(summon.m_amount == 1);
  CHECK(summon.m_side == SummonSide::DEATHRATTLE);
  CHECK(summon.m_addToStack);
  const auto token = Cards::FindCardByID("TB_BaconUps_141t");
  CHECK(token.id == "TB_BaconUps_141t");
  CHECK(token.GetAttack() == 2);
  CHECK(token.GetHealth() == 2);
}
