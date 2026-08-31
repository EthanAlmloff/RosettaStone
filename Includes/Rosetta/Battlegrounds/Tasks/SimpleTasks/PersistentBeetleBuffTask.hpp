#ifndef ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_BEETLE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_BEETLE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds {
class Player;
class Minion;
namespace SimpleTasks {
class PersistentBeetleBuffTask { public:
  PersistentBeetleBuffTask(int attack, int health) : m_attack(attack), m_health(health) {}
  int GetAttack() const noexcept { return m_attack; }
  int GetHealth() const noexcept { return m_health; }
  TaskStatus Run(Player& player, Minion& source);
  TaskStatus Run(Player& player, Minion& source, Minion& target);
 private: int m_attack; int m_health;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds
#endif
