#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>

#include <algorithm>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
GenerateBloodGemsTask::GenerateBloodGemsTask(int amount)
    : m_amount(std::max(0, amount))
{
}

TaskStatus GenerateBloodGemsTask::Run(Player& player, [[maybe_unused]] Minion& source)
{
    player.AddBloodGems(m_amount);
    return TaskStatus::COMPLETE;
}

TaskStatus GenerateBloodGemsTask::Run(Player& player, Minion& source,
                                      [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
