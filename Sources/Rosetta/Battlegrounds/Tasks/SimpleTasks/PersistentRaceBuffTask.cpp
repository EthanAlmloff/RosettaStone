#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion&) { player.ApplyPersistentRaceStats(m_race, m_attack, m_health); return TaskStatus::COMPLETE; }
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
