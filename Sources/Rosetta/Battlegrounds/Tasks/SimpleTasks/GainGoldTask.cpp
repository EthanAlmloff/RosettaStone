#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>

#include <algorithm>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
GainGoldTask::GainGoldTask(int amount, bool nextTurn)
    : m_amount(std::max(0, amount)), m_nextTurn(nextTurn)
{
}

TaskStatus GainGoldTask::Run(Player& player, [[maybe_unused]] Minion& source)
{
    if (m_nextTurn)
    {
        player.season14.AddNextTurnGold(m_amount);
    }
    else
    {
        player.remainCoin += m_amount;
    }
    return TaskStatus::COMPLETE;
}

TaskStatus GainGoldTask::Run(Player& player, Minion& source,
                             [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
