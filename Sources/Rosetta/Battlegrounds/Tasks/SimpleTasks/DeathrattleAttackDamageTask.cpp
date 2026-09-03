#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DeathrattleAttackDamageTask.hpp>
#include <effolkronium/random.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DeathrattleAttackDamageTask::Run(Player& p, Minion& s) {
 if(s.IsDestroyed()||m_count<=0||!p.getOpponentPlayerCallback)return TaskStatus::STOP;
 auto& o=p.getOpponentPlayerCallback(p); std::vector<Minion*> e;
 o.GetField().ForEachAlive([&](MinionData& d){e.push_back(&d.value());});
 for(int i=0;i<m_count&&!e.empty();++i){auto* t=e[effolkronium::random_thread_local::get<std::size_t>(0,e.size()-1)]; t->TakeDamage(s.GetAttack());}
 return TaskStatus::COMPLETE;
}
TaskStatus DeathrattleAttackDamageTask::Run(Player& p,Minion& s,Minion&){return Run(p,s);}
}
