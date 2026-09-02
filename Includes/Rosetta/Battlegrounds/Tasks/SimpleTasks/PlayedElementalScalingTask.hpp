#ifndef ROSETTASTONE_BATTLEGROUNDS_PLAYED_ELEMENTAL_SCALING_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PLAYED_ELEMENTAL_SCALING_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class PlayedElementalScalingTask { public:
 PlayedElementalScalingTask(int attack,int health,int goldenScale):m_attack(attack),m_health(health),m_scale(goldenScale){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_attack,m_health,m_scale;
}; }}
#endif
