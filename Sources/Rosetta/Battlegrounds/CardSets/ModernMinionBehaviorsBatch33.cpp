#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch33.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
namespace {
void AddBuff(std::map<std::string, CardDef>& c, const char* id, Race race, int attack, int health) {
  Power p; p.AddBattlecryTask(SimpleTasks::RandomFriendlyRaceTask{race, attack, health}); c.emplace(id, CardDef{std::move(p)});
}
}
void ModernMinionBehaviorsBatch33::AddAll(std::map<std::string, CardDef>& c) {
  AddBuff(c, "BGS_001", Race::DEMON, 2, 2);
  AddBuff(c, "TB_BaconUps_062", Race::DEMON, 4, 4);
  AddBuff(c, "BGS_038", Race::DRAGON, 2, 2);
  AddBuff(c, "TB_BaconUps_108", Race::DRAGON, 4, 4);
}
}
