#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AfterSellRaceStatsTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus AfterSellRaceStatsTask::Run(Player&, Minion& owner) {
  return TaskStatus::STOP;
}

TaskStatus AfterSellRaceStatsTask::Run(Player&, Minion& owner, Minion& sold) {
  if (m_multiplier <= 0 || !sold.HasRace(m_race)) return TaskStatus::STOP;
  owner.SetAttack(owner.GetAttack() + m_multiplier * sold.GetAttack());
  owner.SetHealth(owner.GetHealth() + m_multiplier * sold.GetHealth());
  return TaskStatus::COMPLETE;
}
}
