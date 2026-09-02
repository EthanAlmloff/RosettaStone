#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class MagnetizationCountBuffTask {
 public:
  MagnetizationCountBuffTask(int attack, int health):m_attack(attack),m_health(health){}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack; int m_health;
};
}}
