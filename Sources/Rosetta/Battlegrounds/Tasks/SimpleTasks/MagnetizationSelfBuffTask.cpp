#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizationSelfBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MagnetizationSelfBuffTask::Run(Player&, Minion& source) {
  const int count = source.GetMagnetizationCount();
  source.SetAttack(source.GetAttack() + count * m_attack);
  source.SetHealth(source.GetHealth() + count * m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus MagnetizationSelfBuffTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
