// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_FREE_REFRESH_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_FREE_REFRESH_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Arms a finite number of free Tavern refreshes for the owning player.
class FreeRefreshTask
{
 public:
    explicit FreeRefreshTask(int amount);

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

 private:
    int m_amount = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_FREE_REFRESH_TASK_HPP
