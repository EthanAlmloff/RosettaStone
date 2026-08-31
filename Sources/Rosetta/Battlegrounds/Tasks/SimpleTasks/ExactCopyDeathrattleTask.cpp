#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ExactCopyDeathrattleTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ExactCopyDeathrattleTask::Run(Player& p, Minion&) {
    auto snapshot = p.season14.TakeExactCopyDeathrattle(m_snapshotId);
    if (!snapshot || !p.isInCombat || p.GetField().IsFull()) return TaskStatus::STOP;
    Minion copy{*snapshot}; p.ApplyFreshMinionModifiers(copy); copy.getPlayerCallback = [&p]() -> Player& { return p; }; if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback()); p.GetField().Add(copy); return TaskStatus::COMPLETE;
}
TaskStatus ExactCopyDeathrattleTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
