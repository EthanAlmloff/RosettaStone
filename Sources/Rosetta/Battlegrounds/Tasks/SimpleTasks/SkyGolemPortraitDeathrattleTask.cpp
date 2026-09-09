#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SkyGolemPortraitDeathrattleTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus SkyGolemPortraitDeathrattleTask::Run(Player& player, Minion&)
{
    bool changed = false;
    player.GetField().ForEachAlive([&changed](MinionData& data) {
        auto& minion = data.value();
        // "Permanently" is a combat-time persistent gain: it must be
        // reconciled back to the matching recruit entity when combat ends.
        // Raw setters would silently drop the grant during reconciliation.
        minion.ApplyCombatPersistentStats(2, 2);
        changed = true;
    });
    return changed ? TaskStatus::COMPLETE : TaskStatus::STOP;
}

TaskStatus SkyGolemPortraitDeathrattleTask::Run(Player& player, Minion& source,
                                                Minion&)
{
    return Run(player, source);
}
}
