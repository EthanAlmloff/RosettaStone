#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSummonFromPoolTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomSummonFromPoolTask::Run(Player& player, Minion& source) {
  if (!player.isInCombat || player.GetField().IsFull()) return TaskStatus::STOP;
  std::vector<const Card*> candidates;
  for (const auto& card : Cards::GetAllCards()) {
    if (!card.isBattlegroundsPoolMinion || card.GetCardType() != CardType::MINION) continue;
    if (m_race != Race::INVALID && m_race != Race::ALL && !card.HasRace(m_race)) continue;
    if (card.GetTier() < m_minTier || (m_maxTier > 0 && card.GetTier() > m_maxTier)) continue;
    if (m_golden && card.premiumDbfID == 0) continue;
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
