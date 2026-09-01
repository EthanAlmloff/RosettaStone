#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackGainHealthTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus AttackGainHealthTask::Run(Player&, Minion&) { return TaskStatus::COMPLETE; }
TaskStatus AttackGainHealthTask::Run(Player& player, Minion& owner, Minion& source)
{
    if (m_health <= 0 || (m_sourceRace != Race::INVALID && !source.HasRace(m_sourceRace))) return TaskStatus::COMPLETE;
    if (player.isInCombat) owner.ApplyCombatPersistentStats(0, m_health);
    else owner.SetHealth(owner.GetHealth() + m_health);
    return TaskStatus::COMPLETE;
}
}
