#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/WindfallTornadoDiscoverTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <algorithm>
#include <effolkronium/random.hpp>
#include <vector>

namespace RosettaStone::Battlegrounds::SimpleTasks {
namespace {
using Random = effolkronium::random_thread_local;

std::vector<Card> ElementalPool() {
  std::vector<Card> result;
  for (const auto& card : Cards::GetAllCards()) {
    if (card.isBattlegroundsPoolMinion && card.hasBehavior &&
        card.normalDbfID == 0 && card.GetCardType() == CardType::MINION &&
        HasActiveTribe(player.activeTribes, card) &&
        card.HasRace(Race::ELEMENTAL))
      result.push_back(card);
  }
  return result;
}
}

TaskStatus WindfallTornadoDiscoverTask::Run(Player& player, Minion& source) {
  if (m_choices <= 0 || player.hand.IsFull() ||
      player.season14.pendingDecision != Season14Decision::NONE)
    return TaskStatus::COMPLETE;
  auto candidates = ElementalPool();
  if (candidates.empty()) return TaskStatus::STOP;
  Random::shuffle(candidates.begin(), candidates.end());
  const auto count = std::min<std::size_t>(3, candidates.size());
  std::vector<Season14Offering> offerings;
  offerings.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
    offerings.push_back({candidates[i].dbfID, 0});

  player.season14.windfallAttack = source.GetAttack();
  player.season14.windfallHealth = source.GetHealth();
  player.season14.windfallRemaining = std::max(0, m_choices - 1);
  // The sold source is no longer on the board; use a zero source entity so
  // replay validation does not require a non-existent board entity.
  player.season14.BeginOfferingDecision(Season14Decision::DISCOVER, 0,
                                        source.GetDbfID(), std::move(offerings));
  return TaskStatus::COMPLETE;
}

TaskStatus WindfallTornadoDiscoverTask::Run(Player& player, Minion& source,
                                             Minion&) {
  return Run(player, source);
}
}
