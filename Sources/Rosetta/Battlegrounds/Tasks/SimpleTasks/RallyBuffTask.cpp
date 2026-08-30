// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus RallyBuffTask::Run(Player& player, Minion& source)
{
    player.GetField().ForEachAlive([this, &source](MinionData& data) {
        Minion& minion = data.value();
        if (minion.GetIndex() != source.GetIndex())
        {
            minion.SetAttack(minion.GetAttack() + m_attack);
            minion.SetHealth(minion.GetHealth() + m_health);
        }
    });
    return TaskStatus::COMPLETE;
}

TaskStatus RallyBuffTask::Run(Player& player, Minion& source,
                              [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
