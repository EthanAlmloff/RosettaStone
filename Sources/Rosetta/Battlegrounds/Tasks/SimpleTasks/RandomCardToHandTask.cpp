#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <unordered_set>
#include <utility>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomCardToHandTask::Run(Player& player, Minion&) {
  if (m_amount <= 0 || player.hand.IsFull()) return TaskStatus::STOP;
  std::vector<const Card*> candidates;
  std::unordered_set<std::string_view> seen;
  for (const auto& card : Cards::GetAllCards()) {
    if (!card.isBattlegroundsPoolMinion || card.GetCardType() != CardType::MINION) continue;
    // Generated cards come from the normal Battlegrounds pool.  Keep golden
    // entities out even if a future pool loader marks them as pool members;
    // golden creation is handled by the simulator's normal triple path.
    if (card.normalDbfID != 0) continue;
    if (m_race != Race::INVALID && m_race != Race::ALL && !card.HasRace(m_race)) continue;
    if (m_tier > 0 && card.GetTier() != m_tier) continue;
    if (!seen.insert(card.id).second) continue;
    candidates.push_back(&card);
  }
  if (candidates.empty()) return TaskStatus::STOP;
  for (int i = 0; i < m_amount && !player.hand.IsFull(); ++i) {
    const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
    Minion generated{*candidates[index]};
    player.ApplyFreshMinionModifiers(generated);
    player.hand.Add(CardData{std::move(generated)});
  }
  return TaskStatus::COMPLETE;
}
TaskStatus RandomCardToHandTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
