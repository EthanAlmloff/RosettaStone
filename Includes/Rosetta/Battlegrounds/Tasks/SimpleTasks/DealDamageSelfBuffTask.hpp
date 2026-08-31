#ifndef ROSETTASTONE_BATTLEGROUNDS_DEAL_DAMAGE_SELF_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DEAL_DAMAGE_SELF_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks { class DealDamageSelfBuffTask { public: DealDamageSelfBuffTask(int a,int h):m_a(a),m_h(h){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_a,m_h; }; }}
#endif
