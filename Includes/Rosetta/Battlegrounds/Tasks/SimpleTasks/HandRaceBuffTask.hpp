#ifndef ROSETTASTONE_BATTLEGROUNDS_HAND_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HAND_RACE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class HandRaceBuffTask {
 public:
  HandRaceBuffTask(int attack, int health) : m_attack(attack), m_health(health) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack; int m_health;
};
}}
#endif
