#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmFodderRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus ArmFodderRefreshTask::Run(Player& p, Minion&) { p.season14.ArmFodderRefreshes(refreshes, perRefresh); return TaskStatus::COMPLETE; } TaskStatus ArmFodderRefreshTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); } }
