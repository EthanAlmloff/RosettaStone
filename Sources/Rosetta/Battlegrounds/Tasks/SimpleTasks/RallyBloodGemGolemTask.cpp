#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemGolemTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyBloodGemGolemTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
TaskStatus RallyBloodGemGolemTask::Run(Player& player, Minion& source, Minion& target) {
  if (m_multiplier <= 0 || source.GetBloodGemCount() <= 0 || target.IsDestroyed())
    return TaskStatus::STOP;
  const auto [attack, health] = player.season14.BloodGemStats();
  player.season14.ArmBloodGemGolemAttack(
      target.GetIndex(), attack * source.GetBloodGemCount() * m_multiplier,
      health * source.GetBloodGemCount() * m_multiplier);
  return TaskStatus::COMPLETE;
}
}
