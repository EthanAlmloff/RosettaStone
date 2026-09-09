#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HealthGainHealthTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus HealthGainHealthTask::Run(Player&, Minion&) {
  return TaskStatus::COMPLETE;
}

TaskStatus HealthGainHealthTask::Run(Player& player, Minion& owner,
                                     Minion& source) {
  if (m_multiplier <= 0 || owner.IsDestroyed() || source.IsDestroyed())
    return TaskStatus::COMPLETE;
  const int amount = player.LastMinionHealthGain() * m_multiplier;
  if (amount <= 0) return TaskStatus::COMPLETE;
  if (player.isInCombat)
    owner.ApplyCombatPersistentStats(0, amount);
  else
    owner.ApplyPersistentMinionStats(0, amount);
  return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
