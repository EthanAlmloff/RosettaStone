#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentTavernTierBuffTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus PersistentTavernTierBuffTask::Run(Player&p,Minion&) { p.season14.persistentTavernTierAttack += attack; p.season14.persistentTavernTierHealth += health; p.season14.persistentTavernTierMax = maxTier; return TaskStatus::COMPLETE; } TaskStatus PersistentTavernTierBuffTask::Run(Player&p,Minion& source,Minion&) { return Run(p,source); } }
