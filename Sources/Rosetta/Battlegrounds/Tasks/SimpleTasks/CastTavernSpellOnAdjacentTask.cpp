#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellOnAdjacentTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus CastTavernSpellOnAdjacentTask::Run(Player& player, Minion& source) {
  const int target = source.GetZonePosition() + 1;
  if (target < 0 || target >= player.recruitField.GetCount() ||
      player.recruitField[static_cast<std::size_t>(target)].IsDestroyed())
    return TaskStatus::STOP;
  return player.CastTavernSpellFree(m_cardID, m_amount, target)
      ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus CastTavernSpellOnAdjacentTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
