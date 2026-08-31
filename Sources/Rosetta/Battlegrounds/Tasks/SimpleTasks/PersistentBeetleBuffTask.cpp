#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentBeetleBuffTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PersistentBeetleBuffTask::Run(Player& player, Minion&) {
  player.season14.persistentBeetleAttack += m_attack;
  player.season14.persistentBeetleHealth += m_health;
  return TaskStatus::COMPLETE;
}
TaskStatus PersistentBeetleBuffTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
