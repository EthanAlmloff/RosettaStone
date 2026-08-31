#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnStatTransferTask.hpp>
#include <optional>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnStatTransferTask::Run(Player& player, Minion& source) {
  if (m_handTarget) {
    bool applied = false;
    player.hand.ForEach([&](std::optional<CardData>& entry) {
      if (!applied && std::holds_alternative<Minion>(*entry)) {
        auto& target = std::get<Minion>(*entry);
        target.SetAttack(target.GetAttack() + source.GetAttack() * (m_repeats ? m_repeats : 1));
        target.SetHealth(target.GetHealth() + source.GetHealth() * (m_repeats ? m_repeats : 1));
        applied = true;
      }
    });
    return TaskStatus::COMPLETE;
  }
  int golden = 0;
  player.recruitField.ForEachAlive([&](const MinionData& data) { if (data.value().IsGolden()) ++golden; });
  const int repeats = m_repeats ? m_repeats : golden;
  if (repeats <= 0) return TaskStatus::COMPLETE;
  const int index = source.GetIndex();
  for (const int slot : {index - 1, index + 1}) if (slot >= 0 && slot < player.recruitField.GetCount()) {
    auto& target = player.recruitField[static_cast<std::size_t>(slot)];
    if (!target.IsDestroyed()) { target.SetAttack(target.GetAttack() + m_attack * repeats); target.SetHealth(target.GetHealth() + m_health * repeats); }
  }
  return TaskStatus::COMPLETE;
}
TaskStatus EndTurnStatTransferTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
