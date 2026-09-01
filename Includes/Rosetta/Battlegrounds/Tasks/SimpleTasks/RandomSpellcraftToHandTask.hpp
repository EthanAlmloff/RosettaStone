#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_SPELLCRAFT_TO_HAND_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_SPELLCRAFT_TO_HAND_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Adds one random canonical Spellcraft spell to hand, marked temporary for
//! the normal recruit-start Spellcraft expiry lifecycle.
class RandomSpellcraftToHandTask {
 public:
  TaskStatus Run(Player&);
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
};
}}
#endif
