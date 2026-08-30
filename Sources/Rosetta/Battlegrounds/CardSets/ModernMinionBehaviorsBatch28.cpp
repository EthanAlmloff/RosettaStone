#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch28.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRaceBuffTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
namespace {
void AddRallyBuff(std::map<std::string, CardDef>& cards, const char* id, int attack, int health) {
  Power power; power.AddRallyTask(SimpleTasks::RallyBuffTask{attack, health});
  cards.emplace(id, CardDef{std::move(power)});
}
void AddRallyRaceBuff(std::map<std::string, CardDef>& cards, const char* id, Race race, int attack, int health) {
  Power power; power.AddRallyTask(SimpleTasks::RallyRaceBuffTask{race, attack, health});
  cards.emplace(id, CardDef{std::move(power)});
}
}
void ModernMinionBehaviorsBatch28::AddAll(std::map<std::string, CardDef>& cards) {
  AddRallyBuff(cards, "BG33_247", 3, 3);
  AddRallyBuff(cards, "BG33_247_G", 6, 6);
  AddRallyRaceBuff(cards, "BG24_708", Race::PIRATE, 2, 1);
  AddRallyRaceBuff(cards, "BG24_708_G", Race::PIRATE, 4, 2);
}
}
