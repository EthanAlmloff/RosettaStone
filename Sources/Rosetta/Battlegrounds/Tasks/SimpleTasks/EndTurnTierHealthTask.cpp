#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnTierHealthTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnTierHealthTask::Run(Player& p, Minion& s) { if (s.IsDestroyed() || m_multiplier <= 0) return TaskStatus::STOP; s.SetHealth(s.GetHealth()+p.currentTier*m_multiplier); return TaskStatus::COMPLETE; }
TaskStatus EndTurnTierHealthTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
