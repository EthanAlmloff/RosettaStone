#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BuyTavernSpellMurlocTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BuyTavernSpellMurlocTask::Run(Player& player, Minion& source) {
  if (m_limit > 0 && !source.CanUseBuyTrigger(m_limit)) return TaskStatus::COMPLETE;
  if (player.lastBoughtTavernSpellID.empty() || player.hand.IsFull()) return TaskStatus::STOP;
  const Card token = Cards::FindCardByID("BG33_890t");
  if (token.id.empty()) return TaskStatus::STOP;
  Minion taught(token);
  taught.SetTaughtTavernSpell(player.lastBoughtTavernSpellID);
  player.ApplyFreshMinionModifiers(taught);
  player.hand.Add(CardData{std::move(taught)});
  source.ConsumeBuyTrigger();
  return TaskStatus::COMPLETE;
}
TaskStatus BuyTavernSpellMurlocTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
