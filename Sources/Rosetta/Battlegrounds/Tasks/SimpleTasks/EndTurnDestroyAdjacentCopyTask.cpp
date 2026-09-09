#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnDestroyAdjacentCopyTask.hpp>
#include <algorithm>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnDestroyAdjacentCopyTask::Run(Player& p, Minion& source) {
  const int pos = source.GetZonePosition();
  std::vector<std::pair<int, Minion>> copies;
  p.recruitField.ForEachAlive([&](MinionData& data) {
    auto& target = data.value();
    if (!target.HasRace(Race::UNDEAD)) return;
    if (target.GetZonePosition() == pos - 1 ||
        (m_both && target.GetZonePosition() == pos + 1))
      copies.emplace_back(target.GetZonePosition(), target);
  });
  if (copies.empty()) return TaskStatus::STOP;
  // Remove/reinsert from right to left.  Removing the left neighbour first
  // shifts the right neighbour one slot left, which otherwise makes a golden
  // Kel'Thuzad silently skip its right-hand copy.
  std::sort(copies.begin(), copies.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first > rhs.first; });
  for (auto& [targetPos, copy] : copies) {
    if (targetPos < 0 || targetPos >= p.recruitField.GetCount()) continue;
    p.recruitField[targetPos].TakeDamage(p.recruitField[targetPos].GetHealth());
    p.recruitField.Remove(p.recruitField[targetPos]);
    if (!p.recruitField.IsFull()) {
      copy.getPlayerCallback = [&p]() -> Player& { return p; };
      if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback());
      p.recruitField.Add(copy, targetPos);
    }
    // The replacement is part of Kel'Thuzad's destroy resolution. Fire the
    // outside-combat observer only after the copy is back on the board so it
    // receives the same buff as every other surviving friendly minion.
    p.ApplyOutsideCombatDestroyTrinkets();
  }
  return TaskStatus::COMPLETE;
}
TaskStatus EndTurnDestroyAdjacentCopyTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
