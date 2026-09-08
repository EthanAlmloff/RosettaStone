#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DarkGiftDoubleAttackTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DarkGiftDoubleAttackTask::Run(Player&, Minion& source) {
    if (source.IsDestroyed()) return TaskStatus::STOP;
    source.SetAttack(source.GetAttack() * 2);
    return TaskStatus::COMPLETE;
}

TaskStatus DarkGiftDoubleAttackTask::Run(Player& player, Minion& source,
                                         Minion&) {
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
