#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizationCountBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MagnetizationCountBuffTask::Run(Player& player, Minion&) {
  player.recruitField.ForEachAlive([this](MinionData& data) {
    auto& minion = data.value();
    const int count = minion.GetMagnetizationCount();
    minion.SetAttack(minion.GetAttack() + count * m_attack);
    minion.SetHealth(minion.GetHealth() + count * m_health);
  });
  return TaskStatus::COMPLETE;
}
TaskStatus MagnetizationCountBuffTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
