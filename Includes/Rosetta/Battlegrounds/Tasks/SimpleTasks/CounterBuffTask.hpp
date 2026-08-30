#ifndef ROSETTASTONE_BATTLEGROUNDS_COUNTER_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_COUNTER_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class CounterBuffTask { public: CounterBuffTask(int a,int h,bool b):m_attack(a),m_health(h),m_battlecries(b){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); int Attack()const{return m_attack;} int Health()const{return m_health;} bool UsesBattlecries()const{return m_battlecries;} private:int m_attack,m_health; bool m_battlecries;};
} }
#endif
