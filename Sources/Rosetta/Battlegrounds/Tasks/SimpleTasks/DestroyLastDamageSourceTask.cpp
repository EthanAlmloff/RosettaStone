#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DestroyLastDamageSourceTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DestroyLastDamageSourceTask::Run(Player& p, Minion& source) {
  if (source.LastDamageSourceIndex() < 0) return TaskStatus::STOP;
  bool destroyed = false;
  const auto& sourceCardID = source.LastDamageSourceCardID();
  auto find = [&](Player& fieldOwner) {
   fieldOwner.battleField.ForEachAlive([&](MinionData& data) {
    auto& target = data.value();
    // Entity indices are only meaningful within a simulator instance.  Keep
    // the card identity check as well so stale/reused indices cannot destroy
    // an unrelated minion after simultaneous death processing.
    if (!destroyed && target.GetIndex() == source.LastDamageSourceIndex() &&
        target.GetCardID() == sourceCardID) {
      target.DestroyImmediately(); destroyed = true;
    }
  });
  };
  // Leeroy is destroyed by the opposing combat source; never fall back to
  // the friendly field when the opponent callback is unavailable.
  if (p.getOpponentPlayerCallback) find(p.getOpponentPlayerCallback(p));
  return destroyed ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus DestroyLastDamageSourceTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
