#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ExactCopyDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatDestroyAdjacentTask.hpp>
#include <algorithm>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatDestroyAdjacentTask::Run(Player& p, Minion& source) {
    const int pos = source.GetZonePosition(); std::vector<Minion*> targets;
    p.GetField().ForEachAlive([&](MinionData& data) { auto& m=data.value(); if (m.GetZonePosition() == pos-1 || (m_both && m.GetZonePosition() == pos+1)) targets.push_back(&m); });
    if (targets.empty()) return TaskStatus::STOP;
    for (auto* target : targets) { const auto snapshotId = p.season14.ArmExactCopyDeathrattle(*target); source.AddDarkGiftDeathrattleTask(TaskType{ExactCopyDeathrattleTask{snapshotId}}); target->TakeDamage(target->GetHealth()); }
    return TaskStatus::COMPLETE;
}
TaskStatus StartCombatDestroyAdjacentTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
