#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <algorithm>
#include <random>
#include <unordered_set>
#include <effolkronium/random.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
namespace {
using Random = effolkronium::random_thread_local;
void Append(const auto& cards, std::vector<Card>& out, Race race, int lo, int hi) {
  for (const auto& card : cards) {
    if (card.id.empty() || !card.hasBehavior || card.normalDbfID != 0 ||
        card.GetCardType() != CardType::MINION || card.GetTier() < lo ||
        card.GetTier() > hi || (race != Race::INVALID && !card.HasRace(race))) continue;
    out.push_back(card);
  }
}
std::vector<Card> Pool(Race race, int lo, int hi) {
  std::vector<Card> result;
  std::unordered_set<std::int32_t> seen;
  auto append = [&](const auto& cards) {
    std::vector<Card> eligible;
    Append(cards, eligible, race, lo, hi);
    for (const auto& card : eligible)
      if (seen.insert(card.dbfID).second) result.push_back(card);
  };
  append(Cards::GetTier1Minions()); append(Cards::GetTier2Minions());
  append(Cards::GetTier3Minions()); append(Cards::GetTier4Minions());
  append(Cards::GetTier5Minions()); append(Cards::GetTier6Minions());
  append(Cards::GetTier7Minions());
  return result;
}
}
TaskStatus MinionOfferingTask::Run(Player& player, Minion& source) {
  if (m_count <= 0 || player.hand.IsFull() || source.GetIndex() < 0)
    return TaskStatus::COMPLETE;
  // A public Discover/Choice is a pending decision, not a replaceable
  // side-channel.  Preserve the first offering until it is selected or
  // explicitly cleared by the state machine.
  if (player.season14.pendingDecision != Season14Decision::NONE)
    return TaskStatus::COMPLETE;
  if (m_requiresFriendlyRace) {
    bool found = false;
    player.recruitField.ForEachAlive([&](const MinionData& data) {
      if (data.value().GetIndex() != source.GetIndex() &&
          data.value().HasRace(m_race)) found = true;
    });
    if (!found) return TaskStatus::COMPLETE;
  }
  auto candidates = Pool(m_race, std::max(1, m_minTier), std::min(7, m_maxTier));
  if (candidates.empty()) return TaskStatus::COMPLETE;
  Random::shuffle(candidates.begin(), candidates.end());
  const auto count = std::min<std::size_t>(static_cast<std::size_t>(m_count), candidates.size());
  std::vector<Season14Offering> offerings;
  offerings.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
    offerings.push_back({candidates[i].dbfID, 0});
  player.season14.BeginOfferingDecision(Season14Decision::DISCOVER,
      static_cast<std::uint64_t>(source.GetIndex()), source.GetDbfID(),
      std::move(offerings));
  return TaskStatus::COMPLETE;
}
TaskStatus MinionOfferingTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
