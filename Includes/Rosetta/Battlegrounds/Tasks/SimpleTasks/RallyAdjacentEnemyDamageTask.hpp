#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_ADJACENT_ENEMY_DAMAGE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_ADJACENT_ENEMY_DAMAGE_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Rally damage to the declared enemy target and one/both adjacent enemies.
class RallyAdjacentEnemyDamageTask {
 public:
  explicit RallyAdjacentEnemyDamageTask(bool bothAdjacent) : m_bothAdjacent(bothAdjacent) {}
  TaskStatus Run(Player& player, Minion& source, Minion& target);
  TaskStatus Run(Player&, Minion&) { return TaskStatus::STOP; }
 private:
  bool m_bothAdjacent = false;
};
} // namespace SimpleTasks
} // namespace RosettaStone::Battlegrounds

#endif
