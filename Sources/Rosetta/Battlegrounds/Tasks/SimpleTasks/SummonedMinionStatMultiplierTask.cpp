#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonedMinionStatMultiplierTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SummonedMinionStatMultiplierTask::Run(Player&, Minion&) { return TaskStatus::COMPLETE; }
TaskStatus SummonedMinionStatMultiplierTask::Run(Player&, Minion&, Minion& target) {
  if (m_race != Race::INVALID && !target.HasRace(m_race)) return TaskStatus::COMPLETE;
  target.SetAttack(target.GetAttack() * m_attackMultiplier);
  target.SetHealth(target.GetHealth() * m_healthMultiplier);
  return TaskStatus::COMPLETE;
}
}
