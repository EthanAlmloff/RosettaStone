#ifndef ROSETTASTONE_BATTLEGROUNDS_BUY_MINION_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BUY_MINION_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class BuyMinionTask {
 public:
  // multiplier > 0 copies the purchased minion's stats to the owner;
  // otherwise the fixed stats are granted to the owner.  targetStats applies
  // the fixed bonus and multiplier to the purchased minion itself.
  BuyMinionTask(int attack, int health, int multiplier = 0, bool targetStats = false, int maxUses = 0)
      : m_attack(attack), m_health(health), m_multiplier(multiplier), m_targetStats(targetStats), m_maxUses(maxUses) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack = 0, m_health = 0, m_multiplier = 0; bool m_targetStats = false; int m_maxUses = 0;
};
} }
#endif
