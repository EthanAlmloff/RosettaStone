#ifndef ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_GOLEM_DEATHRATTLE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_GOLEM_DEATHRATTLE_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Golemancy's owned deathrattle: snapshot the dying minion's stats.
class DarkGiftGolemDeathrattleTask {
 public:
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
};
}}
#endif
