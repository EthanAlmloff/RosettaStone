#ifndef ROSETTASTONE_BATTLEGROUNDS_MAX_HEALTH_DEATHRATTLE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MAX_HEALTH_DEATHRATTLE_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Gives the source minion's maximum Health to another friendly minion.
class MaxHealthDeathrattleTask {
 public:
  explicit MaxHealthDeathrattleTask(int repeats = 1) : m_repeats(repeats) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
  int Repeats() const noexcept { return m_repeats; }
 private: int m_repeats = 1;
};
}}
#endif
