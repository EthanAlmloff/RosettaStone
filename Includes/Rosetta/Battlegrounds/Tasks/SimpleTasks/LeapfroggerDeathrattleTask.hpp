#ifndef ROSETTASTONE_BATTLEGROUNDS_LEAPFROGGER_DEATHRATTLE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_LEAPFROGGER_DEATHRATTLE_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class LeapfroggerDeathrattleTask {
 public:
  LeapfroggerDeathrattleTask(int attack, int health) : m_attack(attack), m_health(health) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int GetAttack() const noexcept { return m_attack; }
  int GetHealth() const noexcept { return m_health; }
 private:
  int m_attack = 0;
  int m_health = 0;
};
}}
#endif
