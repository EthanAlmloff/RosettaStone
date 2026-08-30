#ifndef ROSETTASTONE_BATTLEGROUNDS_ONE_PER_TYPE_RALLY_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ONE_PER_TYPE_RALLY_BUFF_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Gives one friendly minion of every present Battlegrounds type a permanent
//! stat bonus. A dual-type minion may satisfy both type selections.
class OnePerTypeRallyBuffTask {
 public:
  OnePerTypeRallyBuffTask(int attack, int health, int repeats = 1)
      : m_attack(attack), m_health(health), m_repeats(repeats) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int GetAttack() const { return m_attack; }
  int GetHealth() const { return m_health; }
  int GetRepeats() const { return m_repeats; }
 private:
  int m_attack = 0;
  int m_health = 0;
  int m_repeats = 1;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
