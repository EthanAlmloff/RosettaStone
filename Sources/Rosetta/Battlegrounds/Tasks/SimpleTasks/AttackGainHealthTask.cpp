#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackGainHealthTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus AttackGainHealthTask::Run(Player&, Minion&) { return TaskStatus::COMPLETE; }
TaskStatus AttackGainHealthTask::Run(Player& player, Minion& owner, Minion& source)
{
    if (m_health <= 0 || (m_sourceRace != Race::INVALID && !source.HasRace(m_sourceRace))) return TaskStatus::COMPLETE;
    // Sinestra's text is explicitly combat-only.  The same task is used by
    // the generated CardDef in both phases, so make the phase restriction
    // part of the task rather than accidentally turning a recruit-phase
    // attack gain into a permanent health gain.
    if (!player.isInCombat) return TaskStatus::COMPLETE;
    owner.ApplyCombatPersistentStats(0, m_health);
    return TaskStatus::COMPLETE;
}
}
