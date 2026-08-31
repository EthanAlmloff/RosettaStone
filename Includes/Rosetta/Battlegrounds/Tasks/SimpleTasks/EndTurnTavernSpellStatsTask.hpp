#ifndef ROSETTASTONE_BATTLEGROUNDS_END_TURN_TAVERN_SPELL_STATS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_END_TURN_TAVERN_SPELL_STATS_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class EndTurnTavernSpellStatsTask { public:
 EndTurnTavernSpellStatsTask(int attack,int health):m_attack(attack),m_health(health){}
 TaskStatus Run(Player&,Minion&);
 TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_attack,m_health;
}; }}
#endif
