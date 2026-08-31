#ifndef ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_ADJACENT_BLOOD_GEM_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SPELL_CAST_ADJACENT_BLOOD_GEM_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Plays Blood Gems on each friendly board neighbor of the spell target.
class SpellCastAdjacentBloodGemTask {
 public:
  explicit SpellCastAdjacentBloodGemTask(int amount) : m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int Amount() const noexcept { return m_amount; }
 private:
  int m_amount = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
