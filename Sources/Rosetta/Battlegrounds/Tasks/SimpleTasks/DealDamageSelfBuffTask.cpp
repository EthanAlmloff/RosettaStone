#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DealDamageSelfBuffTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus DealDamageSelfBuffTask::Run(Player&,Minion& s){if(s.IsDestroyed())return TaskStatus::STOP;s.SetAttack(s.GetAttack()+m_a);s.SetHealth(s.GetHealth()+m_h);return TaskStatus::COMPLETE;} TaskStatus DealDamageSelfBuffTask::Run(Player& p,Minion& s,Minion&){return Run(p,s);} }
