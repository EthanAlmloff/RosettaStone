#ifndef ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_RANDOM_POOL_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_RANDOM_POOL_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Resolves the bounded random pools used by Patch 36.4 Dark Gifts.
class DarkGiftRandomPoolTask {
 public:
  enum class Pool { MOST_COMMON_RACE_MINION, TAVERN_SPELL };
  explicit DarkGiftRandomPoolTask(Pool pool) : m_pool(pool) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  Pool GetPool() const noexcept { return m_pool; }
 private:
  Pool m_pool;
};
}}
#endif
