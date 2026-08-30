#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch30.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds {
namespace {
void AddEnchant(std::map<std::string, CardDef>& cards, const char* id, int attack, int health) {
  Power p; std::vector<Effect> e; if (attack) e.emplace_back(Effects::AttackN(attack)); if (health) e.emplace_back(Effects::HealthN(health)); p.AddEnchant(Enchant{std::move(e)}); cards.emplace(id, CardDef{std::move(p)});
}
void AddStatic(std::map<std::string, CardDef>& cards, const char* id) { cards.emplace(id, CardDef{}); }
void AddDeathRace(std::map<std::string, CardDef>& cards, const char* id, const char* enchant, Race race) { Power p; p.AddDeathrattleTask(SimpleTasks::FriendlyRaceEnchantmentTask{enchant, race, false}); cards.emplace(id, CardDef{std::move(p)}); }
void AddDeathAll(std::map<std::string, CardDef>& cards, const char* id, const char* enchant) { Power p; p.AddDeathrattleTask(SimpleTasks::AddEnchantmentTask{enchant, EntityType::MINIONS}); cards.emplace(id, CardDef{std::move(p)}); }
void AddDeathSummon(std::map<std::string, CardDef>& cards, const char* id, const char* token, int count) { Power p; p.AddDeathrattleTask(SimpleTasks::SummonTask{token, count}); cards.emplace(id, CardDef{std::move(p)}); }
void AddDeathCard(std::map<std::string, CardDef>& cards, const char* id, const char* card, int count) { Power p; p.AddDeathrattleTask(SimpleTasks::AddCardTask{card, count}); cards.emplace(id, CardDef{std::move(p)}); }
}
void ModernMinionBehaviorsBatch30::AddAll(std::map<std::string, CardDef>& cards) {
  AddStatic(cards, "BG25_050"); AddStatic(cards, "BG25_050_G");
  AddEnchant(cards, "BG28_304e", 4, 6); AddDeathRace(cards, "BG28_304", "BG28_304e", Race::UNDEAD);
  AddEnchant(cards, "BG28_304Ge", 8, 12); AddDeathRace(cards, "BG28_304_G", "BG28_304Ge", Race::UNDEAD);
  AddEnchant(cards, "BG28_306e", 1, 1); AddDeathAll(cards, "BG28_306", "BG28_306e");
  AddEnchant(cards, "BG28_306Ge", 2, 2); AddDeathAll(cards, "BG28_306_G", "BG28_306Ge");
  AddDeathSummon(cards, "BGS_014", "BG_BRM_006t", 1); AddDeathSummon(cards, "TB_BaconUps_113", "TB_BaconUps_030t", 1);
}
}
