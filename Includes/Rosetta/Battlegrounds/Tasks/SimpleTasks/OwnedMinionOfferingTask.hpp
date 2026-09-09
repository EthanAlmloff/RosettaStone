#ifndef ROSETTASTONE_BATTLEGROUNDS_OWNED_MINION_OFFERING_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_OWNED_MINION_OFFERING_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <cstdint>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Opens a one-option public modal for one already-owned minion.
//!
//! The offering carries the stable entity ID as well as the DBF ID.  This is
//! deliberately separate from MinionOfferingTask: the latter samples the
//! Tavern pool, while this primitive is for effects whose target is an
//! existing player-owned entity (and must never leak Tavern ownership).
class OwnedMinionOfferingTask {
 public:
  explicit OwnedMinionOfferingTask(std::int32_t sourceCardDbfID = 0)
      : m_sourceCardDbfID(sourceCardDbfID) {}
  TaskStatus Run(Player&, Minion& source, Minion& target);
 private:
  std::int32_t m_sourceCardDbfID = 0;
};
}}
#endif
