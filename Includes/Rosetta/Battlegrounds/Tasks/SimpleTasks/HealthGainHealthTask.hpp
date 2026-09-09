#ifndef ROSETTASTONE_BATTLEGROUNDS_HEALTH_GAIN_HEALTH_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HEALTH_GAIN_HEALTH_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;

namespace SimpleTasks {
//! Copies a positive health gain from the event source to the owner.
//!
//! The task is deliberately phase-aware. Recruit gains are persistent on the
//! owned entity; combat gains use the combat-persistent ledger so they survive
//! reconciliation without leaking ordinary damage or temporary health.
class HealthGainHealthTask {
 public:
  explicit HealthGainHealthTask(int multiplier = 1)
      : m_multiplier(multiplier) {}

  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion& source);

 private:
  int m_multiplier = 1;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_HEALTH_GAIN_HEALTH_TASK_HPP
