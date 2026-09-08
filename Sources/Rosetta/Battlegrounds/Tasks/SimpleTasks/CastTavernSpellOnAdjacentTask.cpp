#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellOnAdjacentTask.hpp>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus CastTavernSpellOnAdjacentTask::Run(Player& player, Minion& source) {
  auto& field = player.GetField();
  const int pos = source.GetZonePosition();
  std::vector<int> targets;
  if (m_both && pos > 0 && !field[static_cast<std::size_t>(pos - 1)].IsDestroyed()) targets.push_back(pos - 1);
  if (pos + 1 < field.GetCount() && !field[static_cast<std::size_t>(pos + 1)].IsDestroyed()) targets.push_back(pos + 1);
  if (targets.empty()) return TaskStatus::STOP;
  bool cast = false;
  for (const int target : targets) cast = player.CastTavernSpellFree(m_cardID, m_amount, target) || cast;
  return cast ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus CastTavernSpellOnAdjacentTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
