#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastOnSelfHealthTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus SpellCastOnSelfHealthTask::Run(Player&,Minion& source) { source.SetHealth(source.GetHealth()+health); return TaskStatus::COMPLETE; } TaskStatus SpellCastOnSelfHealthTask::Run(Player&p,Minion& source,Minion& target) { if(source.GetIndex()==target.GetIndex()) return Run(p,source); return TaskStatus::COMPLETE; } }
