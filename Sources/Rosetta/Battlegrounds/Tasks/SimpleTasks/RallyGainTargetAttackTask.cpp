#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyGainTargetAttackTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus RallyGainTargetAttackTask::Run(Player&, Minion&)
{
    // This effect requires the Rally target; fail closed if dispatch omits it.
    return TaskStatus::STOP;
}

TaskStatus RallyGainTargetAttackTask::Run(Player&, Minion& source, Minion& target)
{
    if (m_multiplier <= 0 || &source == &target || source.IsDestroyed() ||
        target.IsDestroyed())
    {
        return TaskStatus::STOP;
    }

    source.SetAttack(source.GetAttack() + target.GetAttack() * m_multiplier);
    return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
