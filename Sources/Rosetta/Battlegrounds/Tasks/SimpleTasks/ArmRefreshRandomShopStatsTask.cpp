#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmRefreshRandomShopStatsTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus ArmRefreshRandomShopStatsTask::Run(Player&p,Minion&) { p.season14.ArmRefreshRandomShopStats(attack,health); return TaskStatus::COMPLETE; } TaskStatus ArmRefreshRandomShopStatsTask::Run(Player&p,Minion& source,Minion&) { return Run(p,source); } }
