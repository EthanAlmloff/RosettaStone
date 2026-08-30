#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HandRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus HandRaceBuffTask::Run(Player&, Minion& owner) { owner.SetAttack(owner.GetAttack() + m_attack); owner.SetHealth(owner.GetHealth() + m_health); return TaskStatus::COMPLETE; }
TaskStatus HandRaceBuffTask::Run(Player& p, Minion& owner, Minion&) { return Run(p, owner); }
}
