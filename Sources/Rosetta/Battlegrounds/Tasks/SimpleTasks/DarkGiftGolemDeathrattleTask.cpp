#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DarkGiftGolemDeathrattleTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DarkGiftGolemDeathrattleTask::Run(Player& player, Minion& source) {
  if (player.GetField().IsFull()) return TaskStatus::STOP;
  // The pinned 36.4 snapshot's generic Golem token is Blood Golem; Damaged
  // Golem is a separate Harvest Golem deathrattle token and is not equivalent.
  const Card card = Cards::FindCardByID("BG30_MagicItem_442t");
  if (card.id.empty()) return TaskStatus::STOP;
  Minion summoned{card};
  summoned.SetAttack(source.GetAttack());
  summoned.SetHealth(source.GetHealth());
  player.ApplyFreshMinionModifiers(summoned);
  summoned.getPlayerCallback = [&player]() -> Player& { return player; };
  if (player.getNextCardIndexCallback) summoned.SetIndex(player.getNextCardIndexCallback());
  const int requestedPos = source.GetLastFieldPos() < 0 ? player.GetField().GetCount() : source.GetLastFieldPos();
  const int insertPos = requestedPos > player.GetField().GetCount() ? player.GetField().GetCount() : requestedPos;
  player.GetField().Add(summoned, insertPos);
  Minion& added = player.GetField()[insertPos];
  player.GetField().ForEachAlive([&added](MinionData& data) {
    data.value().ActivateTrigger(TriggerType::SUMMON, added);
  });
  player.ApplySummonTrinkets(added);
  return TaskStatus::COMPLETE;
}
TaskStatus DarkGiftGolemDeathrattleTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
