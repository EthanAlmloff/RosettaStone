#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion&) { for (int i=0;i<p.recruitField.GetCount();++i) { auto& m=p.recruitField[i]; if (!m.IsDestroyed() && m.HasDeathrattle()) { m.ActivateTask(PowerType::DEATHRATTLE,p); return TaskStatus::COMPLETE; } } return TaskStatus::STOP; } TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); } }
