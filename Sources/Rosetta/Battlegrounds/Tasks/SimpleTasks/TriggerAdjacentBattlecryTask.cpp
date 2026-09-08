#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerAdjacentBattlecryTask.hpp>

#include <array>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus TriggerAdjacentBattlecryTask::Run(Player& player, Minion& source)
{
    auto& field = player.isInCombat ? player.battleField : player.recruitField;
    const int removed = source.GetLastFieldPos();
    std::array<Minion*, 2> adjacent{nullptr, nullptr};
    field.ForEachAlive([&](MinionData& data) {
        auto& candidate = data.value();
        if (candidate.GetZonePosition() == removed - 1)
            adjacent[0] = &candidate;
        else if (candidate.GetZonePosition() == removed)
            adjacent[1] = &candidate;
    });

    const int repeats = m_golden ? 2 : 1;
    bool triggered = false;
    for (auto* target : adjacent)
    {
        if (target == nullptr || !target->HasBattlecry()) continue;
        triggered = true;
        for (int repeat = 0; repeat < repeats; ++repeat)
        {
            target->ActivateTask(PowerType::POWER, player);
            if (player.season14.pendingDecision != Season14Decision::NONE)
                return TaskStatus::COMPLETE;
        }
    }
    return triggered ? TaskStatus::COMPLETE : TaskStatus::STOP;
}

TaskStatus TriggerAdjacentBattlecryTask::Run(Player& player, Minion& source,
                                             Minion&)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
