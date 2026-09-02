#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PlayedElementalScalingTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PlayedElementalScalingTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
TaskStatus PlayedElementalScalingTask::Run(Player& p, Minion&, Minion& target) {
 const int n=p.season14.minionsPlayedThisTurn;
 target.ApplyPersistentMinionStats(m_attack + n*m_scale, m_health + n*m_scale);
 return TaskStatus::COMPLETE;
}
}
