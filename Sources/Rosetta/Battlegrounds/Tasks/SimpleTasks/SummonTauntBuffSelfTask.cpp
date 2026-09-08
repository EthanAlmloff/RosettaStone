#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTauntBuffSelfTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus SummonTauntBuffSelfTask::Run(Player&, Minion& owner,
                                        Minion& summoned)
{
    if (summoned.HasTaunt())
        owner.ApplyPersistentMinionStats(m_attack, m_health);
    return TaskStatus::COMPLETE;
}

TaskStatus SummonTauntBuffSelfTask::Run(Player&, Minion&)
{
    return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
