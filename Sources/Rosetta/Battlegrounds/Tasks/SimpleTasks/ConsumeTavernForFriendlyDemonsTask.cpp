#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeTavernForFriendlyDemonsTask.hpp>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ConsumeTavernForFriendlyDemonsTask::Run(Player& player, Minion&) {
  if (m_multiplier <= 0) return TaskStatus::STOP;
  std::vector<Minion*> demons;
  player.GetField().ForEachAlive([&](MinionData& data) {
    if (data.value().HasRace(Race::DEMON)) demons.push_back(&data.value());
  });
  bool consumed = false;
  for (Minion* demon : demons)
    consumed = ConsumeRandomTavernTask{m_multiplier}.Run(player, *demon) == TaskStatus::COMPLETE || consumed;
  return consumed ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus ConsumeTavernForFriendlyDemonsTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
