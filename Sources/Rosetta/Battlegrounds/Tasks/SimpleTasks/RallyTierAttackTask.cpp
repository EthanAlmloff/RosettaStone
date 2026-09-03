#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyTierAttackTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyTierAttackTask::Run(Player& p, Minion& s) { if (s.IsDestroyed() || m_multiplier <= 0) return TaskStatus::STOP; s.SetAttack(s.GetAttack()+p.currentTier*m_multiplier); return TaskStatus::COMPLETE; }
TaskStatus RallyTierAttackTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
