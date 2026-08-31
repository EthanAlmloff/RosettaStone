#ifndef ROSETTASTONE_BATTLEGROUNDS_CAST_SPELL_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_CAST_SPELL_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
enum class MinionBuffTarget { FRIENDLY_BOARD_RACE, FRIENDLY_BOARD_AND_HAND_RACE, LEFTMOST_HAND, TAVERN_RACE_PERSISTENT, TAVERN_ALL_PERSISTENT };
class ApplyMinionStatBuffTask { public:
 ApplyMinionStatBuffTask(Race race,int attack,int health,MinionBuffTarget target):m_race(race),m_attack(attack),m_health(health),m_target(target){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
private: Race m_race; int m_attack; int m_health; MinionBuffTarget m_target;
}; }}
#endif
