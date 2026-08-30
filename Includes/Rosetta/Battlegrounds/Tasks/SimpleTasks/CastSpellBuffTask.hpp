#ifndef ROSETTASTONE_BATTLEGROUNDS_CAST_SPELL_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_CAST_SPELL_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class CastSpellBuffTask { public:
 CastSpellBuffTask(Race race,int attack,int health,bool includeHand,int mode=0):m_race(race),m_attack(attack),m_health(health),m_includeHand(includeHand),m_mode(mode){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
private: Race m_race; int m_attack; int m_health; bool m_includeHand; int m_mode;
}; }}
#endif
