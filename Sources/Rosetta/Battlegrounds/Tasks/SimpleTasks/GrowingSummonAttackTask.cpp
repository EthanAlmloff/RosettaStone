#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GrowingSummonAttackTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus GrowingSummonAttackTask::Run(Player&, Minion&) { return TaskStatus::COMPLETE; }
TaskStatus GrowingSummonAttackTask::Run(Player& player, Minion& owner, Minion& target) {
  // ALL is a selector wildcard.  Minion::HasRace treats ALL as an actual
  // card race, so handle the wildcard here rather than accidentally
  // excluding every ordinary summoned minion.
  if (m_race != Race::INVALID && m_race != Race::ALL && !target.HasRace(m_race)) return TaskStatus::COMPLETE;
  const int bonus = owner.GetIndex() >= 0
      ? player.season14.TakeGrowingSummonAttack(owner.GetIndex(), m_initialAttack, m_increment)
      : m_nextAttack;
  if (owner.GetIndex() < 0) m_nextAttack += m_increment;
  target.SetAttack(target.GetAttack() + bonus);
  return TaskStatus::COMPLETE;
}
}
