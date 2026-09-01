#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomElementalHandAndSummonTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
#include <utility>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomElementalHandAndSummonTask::Run(Player& p, Minion&) {
  if (m_amount <= 0) return TaskStatus::STOP;
  std::vector<const Card*> pool;
  for (const auto& card : Cards::GetAllCards())
    if (card.isBattlegroundsPoolMinion && card.hasBehavior && card.normalDbfID == 0 &&
        card.GetCardType() == CardType::MINION && card.HasRace(Race::ELEMENTAL)) pool.push_back(&card);
  if (pool.empty()) return TaskStatus::STOP;
  for (int i = 0; i < m_amount && !p.hand.IsFull(); ++i) {
    const auto& card = *pool[Random::get<std::size_t>(0, pool.size() - 1)];
    // Hand and combat summon are distinct instances.  Never let a full hand
    // suppress the independent summon, and give each copy a fresh identity.
    Minion handCopy{card};
    p.ApplyFreshMinionModifiers(handCopy);
    if (!p.hand.IsFull()) p.hand.Add(CardData{std::move(handCopy)});
    Minion combatCopy{card};
    p.SummonCombatSnapshot(std::move(combatCopy));
  }
  return TaskStatus::COMPLETE;
}
TaskStatus RandomElementalHandAndSummonTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
