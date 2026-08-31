// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemOtherTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyBloodGemOtherTask::Run(Player& player, Minion& source) {
  if (m_amount <= 0 || source.IsDestroyed()) return TaskStatus::STOP;
  player.GetField().ForEachAlive([&](MinionData& data) {
    auto& target = data.value();
    // Entity indexes are authoritative in a live lobby, but focused/unit
    // callers may use unindexed instances.  Pointer identity keeps the
    // source excluded in both cases without excluding every -1-index card.
    if (&target == &source ||
        (source.GetIndex() >= 0 && target.GetIndex() == source.GetIndex()))
        return;
    const auto [attack, health] = player.season14.BloodGemStatsFor(target.GetRace());
    for (int i = 0; i < m_amount; ++i) target.ApplyBloodGem(attack, health);
  });
  return TaskStatus::COMPLETE;
}
TaskStatus RallyBloodGemOtherTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
