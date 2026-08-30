// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_BUFF_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Applies a Rally's temporary combat stat bonus to the owner's other minions.
class RallyBuffTask
{
 public:
    explicit RallyBuffTask(int attack, int health)
        : m_attack(attack), m_health(health)
    {
    }

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

 private:
    int m_attack = 0;
    int m_health = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_RALLY_BUFF_TASK_HPP
