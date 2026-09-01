#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonRecentDeadMinionsTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SummonRecentDeadMinionsTask::Run(Player& p, Minion&) {
  if (m_count <= 0) return TaskStatus::STOP;
  auto snapshots = p.season14.TakeCombatDeadMinions(m_race, static_cast<std::size_t>(m_count));
  const bool had = !snapshots.empty();
  for (auto& snapshot : snapshots) if (!p.SummonCombatSnapshot(std::move(snapshot))) break;
  return had ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus SummonRecentDeadMinionsTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
