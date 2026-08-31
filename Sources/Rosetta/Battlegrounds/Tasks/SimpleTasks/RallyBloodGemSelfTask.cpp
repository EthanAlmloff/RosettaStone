// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemSelfTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus RallyBloodGemSelfTask::Run(Player& player, Minion& source)
{
    if (m_amount <= 0 || source.IsDestroyed()) return TaskStatus::STOP;
    const auto [attack, health] = player.season14.BloodGemStatsFor(source.GetRace());
    for (int i = 0; i < m_amount; ++i)
        source.ApplyBloodGem(attack, health);
    return TaskStatus::COMPLETE;
}

TaskStatus RallyBloodGemSelfTask::Run(Player& player, Minion& source,
                                      [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
