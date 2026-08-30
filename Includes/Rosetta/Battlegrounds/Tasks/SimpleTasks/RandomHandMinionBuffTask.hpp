#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_HAND_MINION_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_HAND_MINION_BUFF_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Gives one random minion in the owner's hand a permanent stat bonus.
//! Empty/full hand and non-minion cards resolve as a no-op.  Selection uses
//! RosettaStone's seeded RNG stream, so replay remains deterministic.
class RandomHandMinionBuffTask {
 public:
  RandomHandMinionBuffTask(int attack, int health)
      : m_attack(attack), m_health(health) {}
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
