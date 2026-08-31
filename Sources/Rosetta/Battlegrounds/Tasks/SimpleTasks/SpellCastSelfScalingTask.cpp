#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastSelfScalingTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SpellCastSelfScalingTask::Run(Player&, Minion& source)
{
    source.SetAttack(source.GetAttack() + attack);
    source.SetHealth(source.GetHealth() + health);
    return TaskStatus::COMPLETE;
}

TaskStatus SpellCastSelfScalingTask::Run(Player& player, Minion& source,
                                         Minion& target)
{
    if (source.GetIndex() == target.GetIndex()) return Run(player, source);
    return TaskStatus::COMPLETE;
}
}
