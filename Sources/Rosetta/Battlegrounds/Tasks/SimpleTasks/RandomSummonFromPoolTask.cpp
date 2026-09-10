#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSummonFromPoolTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomSummonFromPoolTask::Run(Player& player, Minion& source) {
  if (!player.isInCombat) return TaskStatus::STOP;
  if (player.GetField().IsFull()) {
    player.ApplySummonOverflowTrinkets();
    return TaskStatus::STOP;
  }
  std::vector<const Card*> candidates;
  // Sneed's Replicator is the one reviewed generated child whose printed
  // range is relative to the owner's current Tavern tier rather than the
  // fixed [1, 6] range encoded in its CardDef.  The starting Shredder now
  // carries this child task, so resolve the relative upper bound here and do
  // not also run Player's bespoke fallback resolver for that instance.
  const int effectiveMaxTier =
      (source.GetCardID() == "BG21_HERO_030t" && player.currentTier > 0)
          ? player.currentTier - 1
          : m_maxTier;
  for (const auto& card : Cards::GetAllCards()) {
    if (!card.isBattlegroundsPoolMinion || card.GetCardType() != CardType::MINION) continue;
    if (m_race != Race::INVALID && m_race != Race::ALL && !card.HasRace(m_race)) continue;
    if (card.GetTier() < m_minTier ||
        (effectiveMaxTier > 0 && card.GetTier() > effectiveMaxTier)) continue;
    if (m_golden && card.premiumDbfID == 0) continue;
    if (m_battlecryOnly && (!card.gameTags.contains(GameTag::BATTLECRY) ||
                            card.gameTags.at(GameTag::BATTLECRY) == 0)) continue;
    candidates.push_back(&card);
  }
  if (candidates.empty()) return TaskStatus::STOP;
  const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
  Minion summoned{*candidates[index]};
  if (m_golden && !summoned.MakeGolden()) return TaskStatus::STOP;
  player.ApplyFreshMinionModifiers(summoned);
  summoned.SetAttack(m_stat); summoned.SetHealth(m_stat);
  summoned.getPlayerCallback = [&player]() -> Player& { return player; };
  if (player.getNextCardIndexCallback) summoned.SetIndex(player.getNextCardIndexCallback());
  int position = source.GetLastFieldPos();
  if (position < 0 || position > player.GetField().GetCount()) position = player.GetField().GetCount();
  player.GetField().Add(summoned, position);
  const int addedPosition = position < player.GetField().GetCount() ? position : player.GetField().GetCount() - 1;
  Minion& added = player.GetField()[addedPosition];
  player.GetField().ForEachAlive([&added](MinionData& data) { data.value().ActivateTrigger(TriggerType::SUMMON, added); });
  player.ApplySummonTrinkets(added);
  return TaskStatus::COMPLETE;
}
TaskStatus RandomSummonFromPoolTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
