#ifndef ROSETTASTONE_BATTLEGROUNDS_ATTACKING_MINION_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ATTACKING_MINION_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class AttackingMinionBuffTask { public:
 AttackingMinionBuffTask(int attack,int health, Race triggerRace = Race::INVALID):m_attack(attack),m_health(health),m_triggerRace(triggerRace){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 Race GetTriggerRace() const { return m_triggerRace; }
 int GetAttack() const { return m_attack; }
 int GetHealth() const { return m_health; }
private:int m_attack;int m_health; Race m_triggerRace = Race::INVALID;
}; }}
#endif
