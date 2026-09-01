#ifndef ROSETTASTONE_BATTLEGROUNDS_QUILBOAR_BLOOD_GOLEM_DEATHRATTLE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_QUILBOAR_BLOOD_GOLEM_DEATHRATTLE_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class QuilboarBloodGolemDeathrattleTask {
 public:
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
};
}}
#endif
