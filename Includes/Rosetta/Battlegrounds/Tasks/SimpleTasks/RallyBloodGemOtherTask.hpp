// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_BLOOD_GEM_OTHER_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_BLOOD_GEM_OTHER_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Plays the resolved Blood Gem amount on every other friendly minion.
class RallyBloodGemOtherTask { public:
  explicit RallyBloodGemOtherTask(int amount) : m_amount(amount) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_amount = 0;
};
} }
#endif
