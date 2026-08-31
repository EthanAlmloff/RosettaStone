#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConfigureAttackThresholdTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus ConfigureAttackThresholdTask::Run(Player&,Minion& s){s.SetAttackThresholdDivineShield(threshold);return TaskStatus::COMPLETE;} TaskStatus ConfigureAttackThresholdTask::Run(Player&p,Minion&s,Minion&){return Run(p,s);} }
