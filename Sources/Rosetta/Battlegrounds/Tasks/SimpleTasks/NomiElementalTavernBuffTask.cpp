#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/NomiElementalTavernBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus NomiElementalTavernBuffTask::Run(Player& p, Minion&) {
  if (m_amount <= 0) return TaskStatus::STOP;
  p.ApplyTavernRaceBuff(Race::ELEMENTAL, m_amount, m_amount);
  return TaskStatus::COMPLETE;
}
TaskStatus NomiElementalTavernBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
