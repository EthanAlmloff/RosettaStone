#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_MAGNETIC_MECH_TO_TARGET_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_MAGNETIC_MECH_TO_TARGET_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; }
namespace RosettaStone::Battlegrounds::SimpleTasks {
class RandomMagneticMechToTargetTask {
 public:
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion& target);
};
}
#endif
