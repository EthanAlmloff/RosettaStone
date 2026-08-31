// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_BLOOD_GEM_SELF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_BLOOD_GEM_SELF_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Plays the resolved Blood Gem amount on the Rally source permanently.
class RallyBloodGemSelfTask
{
 public:
    explicit RallyBloodGemSelfTask(int amount) : m_amount(amount) {}

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

 private:
    int m_amount = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_RALLY_BLOOD_GEM_SELF_TASK_HPP
