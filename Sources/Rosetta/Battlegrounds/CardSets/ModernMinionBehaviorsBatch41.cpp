#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch41.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChromadrakeToHandTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch41::AddAll(std::map<std::string, CardDef>& cards) {
  auto beasts = [](const char* id, int count) { Power p; p.AddRallyTask(SimpleTasks::RandomCardToHandTask{Race::BEAST, 0, count}); return std::pair{id, CardDef{std::move(p)}}; };
  auto dragons = [](const char* id, int count) { Power p; p.AddRallyTask(SimpleTasks::RandomChromadrakeToHandTask{count}); return std::pair{id, CardDef{std::move(p)}}; };
  cards.insert(beasts("BG36_204", 1)); cards.insert(beasts("BG36_204_G", 2));
  cards.insert(dragons("BG36_242", 1)); cards.insert(dragons("BG36_242_G", 2));
}
}
