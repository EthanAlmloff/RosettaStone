#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch30.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Card.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch30] - static keywords and fixed effects") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch30::AddAll(cards);
  CHECK_EQ(cards.size(), 14);
  for (const auto id : {"BG25_050", "BG25_050_G", "BG28_304", "BG28_304_G", "BG28_306", "BG28_306_G", "BGS_014", "TB_BaconUps_113", "BG36_854", "BG36_854_G"}) REQUIRE(cards.contains(id));
  for (const auto id : {"BG25_050", "BG25_050_G"}) { Minion m{Cards::FindCardByID(id)}; CHECK(m.HasDivineShield()); CHECK(m.HasReborn()); }
  CHECK(std::holds_alternative<SimpleTasks::SummonTask>(cards.at("BGS_014").power.GetDeathrattleTask().front()));
  CHECK(std::holds_alternative<SimpleTasks::AddCardTask>(cards.at("BG36_854").power.GetDeathrattleTask().front()));
}

TEST_CASE("[Batch30] - deathrattle payloads match pinned metadata") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch30::AddAll(cards);
  const auto checkSummon = [&cards](const char* id, const char* token, int amount) {
    const auto& tasks = cards.at(id).power.GetDeathrattleTask();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::SummonTask>(tasks.front()));
    const auto& task = std::get<SimpleTasks::SummonTask>(tasks.front());
    CHECK(task.m_cardID == token);
    CHECK(task.m_amount == amount);
  };
  checkSummon("BGS_014", "BG_BRM_006t", 1);
  checkSummon("TB_BaconUps_113", "TB_BaconUps_030t", 1);

  const auto checkAddCard = [&cards](const char* id, int amount) {
    const auto& tasks = cards.at(id).power.GetDeathrattleTask();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::AddCardTask>(tasks.front()));
    const auto& task = std::get<SimpleTasks::AddCardTask>(tasks.front());
    CHECK(task.CardID() == "BG36_624");
    CHECK(task.Amount() == amount);
  };
  checkAddCard("BG36_854", 1);
  checkAddCard("BG36_854_G", 2);
}

TEST_CASE("[Batch30] - race and all-minion selectors are exact") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch30::AddAll(cards);
  for (const auto* id : {"BG28_304", "BG28_304_G"}) {
    const auto& tasks = cards.at(id).power.GetDeathrattleTask();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::FriendlyRaceEnchantmentTask>(tasks.front()));
  }
  const auto& moroes = std::get<SimpleTasks::FriendlyRaceEnchantmentTask>(
      cards.at("BG28_304").power.GetDeathrattleTask().front());
  CHECK(moroes.CardID() == "BG28_304e");
  CHECK(moroes.GetRace() == Race::UNDEAD);
  CHECK(!moroes.ExcludesSource());

  for (const auto* id : {"BG28_306", "BG28_306_G"}) {
    const auto& tasks = cards.at(id).power.GetDeathrattleTask();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(tasks.front()));
    const auto& task = std::get<SimpleTasks::AddEnchantmentTask>(tasks.front());
    CHECK(task.Entity() == EntityType::MINIONS);
  }
}

TEST_CASE("[Batch30] - pinned static keywords and payload card types") {
  const auto checkMinion = [](const char* id, int attack, int health, Race race,
                              bool taunt, bool reborn, bool shield) {
    const auto card = Cards::FindCardByID(id);
    REQUIRE(card.id == id);
    CHECK(card.GetAttack() == attack);
    CHECK(card.GetHealth() == health);
    CHECK(card.GetRace() == race);
    Minion minion{card};
    CHECK(minion.HasTaunt() == taunt);
    CHECK(minion.HasReborn() == reborn);
    CHECK(minion.HasDivineShield() == shield);
  };
  checkMinion("BG25_050", 9, 9, Race::UNDEAD, false, true, true);
  checkMinion("BG25_050_G", 18, 18, Race::UNDEAD, false, true, true);
  checkMinion("BG28_304", 6, 4, Race::UNDEAD, false, true, false);
  checkMinion("BG28_304_G", 12, 8, Race::UNDEAD, false, true, false);
  checkMinion("BG28_306", 3, 1, Race::UNDEAD, false, true, false);
  checkMinion("BG28_306_G", 6, 2, Race::UNDEAD, false, true, false);
  checkMinion("BGS_014", 3, 2, Race::DEMON, true, false, false);
  checkMinion("TB_BaconUps_113", 6, 4, Race::DEMON, true, false, false);
  checkMinion("BG36_854", 2, 1, Race::MECHANICAL, true, false, false);
  checkMinion("BG36_854_G", 4, 2, Race::MECHANICAL, true, false, false);

  CHECK(Cards::FindCardByID("BG_BRM_006t").GetCardType() == CardType::MINION);
  CHECK(Cards::FindCardByID("TB_BaconUps_030t").GetCardType() == CardType::MINION);
  CHECK(Cards::FindCardByID("BG36_624").GetCardType() == CardType::BATTLEGROUND_SPELL);
}
