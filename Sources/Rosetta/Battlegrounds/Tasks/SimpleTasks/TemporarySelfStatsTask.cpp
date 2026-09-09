#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TemporarySelfStatsTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus TemporarySelfStatsTask::Run(Player&, Minion& owner) {
  if (m_attack == 0 && m_health == 0) return TaskStatus::COMPLETE;
  owner.ApplyTemporaryEnchantment(Minion::TemporaryEnchantment::Stats,
                                  m_attack, m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus TemporarySelfStatsTask::Run(Player& player, Minion& owner, Minion&) {
  return Run(player, owner);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
