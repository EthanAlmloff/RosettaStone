#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemAttackerTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyBloodGemAttackerTask::Run(Player& player, Minion& source) {
  if (m_amount <= 0 || source.IsDestroyed()) return TaskStatus::STOP;
  for (int i = 0; i < m_amount; ++i) player.ApplyBloodGemTo(source);
  return TaskStatus::COMPLETE;
}

TaskStatus RallyBloodGemAttackerTask::Run(Player& player, Minion& source,
                                          Minion&) {
  return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
