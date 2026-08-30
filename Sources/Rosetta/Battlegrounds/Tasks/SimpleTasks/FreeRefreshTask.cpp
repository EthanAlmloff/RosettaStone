// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
FreeRefreshTask::FreeRefreshTask(int amount) : m_amount(amount)
{
    // Do nothing
}

TaskStatus FreeRefreshTask::Run(Player& player,
                                [[maybe_unused]] Minion& source)
{
    player.season14.AddFreeRefreshes(m_amount);
    return TaskStatus::COMPLETE;
}

TaskStatus FreeRefreshTask::Run(
    Player& player, Minion& source, [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
