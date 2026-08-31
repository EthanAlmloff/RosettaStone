#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastAdjacentBloodGemTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SpellCastAdjacentBloodGemTask::Run(Player& player, Minion& source) {
  if (m_amount <= 0 || source.IsDestroyed()) return TaskStatus::STOP;
  const auto center = source.GetZonePosition();
  if (center < 0) return TaskStatus::STOP;
  for (int offset : {-1, 1}) {
    const int index = center + offset;
    if (index < 0 || index >= player.recruitField.GetCount()) continue;
    auto& target = player.recruitField[static_cast<std::size_t>(index)];
    if (target.IsDestroyed()) continue;
    // Resolve through the same canonical path as a Blood Gem spell.  Besides
    // the base amount, this applies race auras and downstream gem triggers
    // (Agamaggan, Tough Tusk, Dynamic Duo, etc.) consistently.
    for (int i = 0; i < m_amount; ++i) player.ApplyBloodGemTo(target);
  }
  return TaskStatus::COMPLETE;
}

TaskStatus SpellCastAdjacentBloodGemTask::Run(Player& player, Minion& source,
                                              Minion&) {
  return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
