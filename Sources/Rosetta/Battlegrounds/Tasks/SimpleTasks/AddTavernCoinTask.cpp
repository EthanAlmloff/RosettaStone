#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddTavernCoinTask.hpp>

#include <algorithm>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
AddTavernCoinTask::AddTavernCoinTask(int amount)
    : m_amount(std::max(0, amount))
{
}

TaskStatus AddTavernCoinTask::Run(Player& player, [[maybe_unused]] Minion& source)
{
    player.AddTavernCoins(m_amount);
    return TaskStatus::COMPLETE;
}

TaskStatus AddTavernCoinTask::Run(Player& player, Minion& source,
                                  [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
