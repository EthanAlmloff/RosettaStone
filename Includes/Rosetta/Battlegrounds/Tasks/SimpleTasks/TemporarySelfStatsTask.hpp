#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
class TemporarySelfStatsTask {
 public:
  TemporarySelfStatsTask(int attack, int health)
      : m_attack(attack), m_health(health) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  int m_attack = 0;
  int m_health = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds
