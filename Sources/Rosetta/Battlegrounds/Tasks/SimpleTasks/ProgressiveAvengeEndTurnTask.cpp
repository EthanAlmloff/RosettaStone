#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ProgressiveAvengeEndTurnTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ProgressiveAvengeEndTurnTask::Run(Player& p, Minion&) { p.recruitField.ForEachAlive([&](MinionData& d){ d.value().ApplyPersistentMinionStats(p.season14.progressiveAvengeAttack,p.season14.progressiveAvengeHealth); }); return TaskStatus::COMPLETE; }
TaskStatus ProgressiveAvengeEndTurnTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
