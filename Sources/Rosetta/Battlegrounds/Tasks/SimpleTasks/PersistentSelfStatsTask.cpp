#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentSelfStatsTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PersistentSelfStatsTask::Run(Player&, Minion& owner)
{
  // This task is used by repeating end-turn effects.  The payload is an
  // increment on every trigger, not an idempotent all-minion aura: using
  // ApplyPersistentMinionStats here would apply the first +2/+2 and silently
  // suppress every later turn because that API stores a maximum watermark.
  owner.SetAttack(owner.GetAttack() + m_attack);
  owner.SetHealth(owner.GetHealth() + m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus PersistentSelfStatsTask::Run(Player& p, Minion& owner, Minion&){ return Run(p,owner); }
}
