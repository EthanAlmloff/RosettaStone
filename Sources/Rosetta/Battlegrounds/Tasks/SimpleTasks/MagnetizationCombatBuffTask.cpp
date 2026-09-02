#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizationCombatBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MagnetizationCombatBuffTask::Run(Player& player, Minion&) {
  const int attack = m_base + player.magnetizationsThisGame * m_per;
  auto& field = player.isInCombat ? player.battleField : player.recruitField;
  field.ForEachAlive([attack](MinionData& data) {
    auto& minion = data.value();
    if (minion.HasRace(Race::MECHANICAL)) minion.SetAttack(minion.GetAttack() + attack);
  });
  return TaskStatus::COMPLETE;
}
TaskStatus MagnetizationCombatBuffTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
