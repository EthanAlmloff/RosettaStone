#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CounterBuffTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus CounterBuffTask::Run(Player&p,Minion&s){const int n=m_battlecries?p.season14.battlecriesTriggered:p.season14.minionsPlayedThisTurn; s.SetAttack(s.GetAttack()+m_attack*n); s.SetHealth(s.GetHealth()+m_health*n); return TaskStatus::COMPLETE;} TaskStatus CounterBuffTask::Run(Player&p,Minion&s,Minion&){return Run(p,s);} }
