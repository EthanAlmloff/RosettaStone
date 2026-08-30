#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackingMinionBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus AttackingMinionBuffTask::Run(Player&,Minion&) { return TaskStatus::COMPLETE; }
TaskStatus AttackingMinionBuffTask::Run(Player&,Minion& source,Minion& target) { if (m_triggerRace != Race::INVALID && !source.HasRace(m_triggerRace)) return TaskStatus::COMPLETE; target.SetAttack(target.GetAttack()+m_attack); target.SetHealth(target.GetHealth()+m_health); return TaskStatus::COMPLETE; }
}
