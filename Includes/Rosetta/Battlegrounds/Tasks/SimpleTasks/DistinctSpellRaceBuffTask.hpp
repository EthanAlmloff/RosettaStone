#ifndef ROSETTASTONE_BATTLEGROUNDS_DISTINCT_SPELL_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DISTINCT_SPELL_RACE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class DistinctSpellRaceBuffTask { public:
 DistinctSpellRaceBuffTask(int attack,int health,int scale):m_attack(attack),m_health(health),m_scale(scale){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_attack,m_health,m_scale;
}; }}
#endif
