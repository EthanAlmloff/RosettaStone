#ifndef ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_DOUBLE_ATTACK_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_DOUBLE_ATTACK_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Elixir of Vim: double the gifted minion's current attack on Rally.
class DarkGiftDoubleAttackTask {
 public:
    TaskStatus Run(Player&, Minion& source);
    TaskStatus Run(Player&, Minion& source, Minion& target);
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
