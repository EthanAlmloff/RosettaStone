#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTavernSpellToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <utility>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomTavernSpellToHandTask::Run(Player& player, Minion&) { return Run(player); }
TaskStatus RandomTavernSpellToHandTask::Run(Player& player, Minion&, Minion&) { return Run(player); }
TaskStatus RandomTavernSpellToHandTask::Run(Player& player) {
  if (m_amount <= 0 || player.hand.IsFull()) return TaskStatus::STOP;
  std::vector<const Card*> pool;
  for (const auto& card : Cards::GetAllCards()) {
    if (!card.isBattlegroundsPoolSpell) continue;
    if (card.GetCardType() != CardType::SPELL && card.GetCardType() != CardType::BATTLEGROUND_SPELL) continue;
    // Pool entries are normal definitions.  Premium/generated spell rows
    // must not become an accidental second pool or bypass normal creation.
    if (card.normalDbfID != 0) continue;
    const auto costTag = card.gameTags.find(GameTag::COST);
    if (m_maxCost > 0 &&
        (costTag == card.gameTags.end() || costTag->second != m_maxCost))
        continue;
    pool.push_back(&card);
  }
  if (pool.empty()) return TaskStatus::STOP;
  for (int i = 0; i < m_amount && !player.hand.IsFull(); ++i) {
    Spell generated{*pool[Random::get<std::size_t>(0, pool.size() - 1)]};
    player.hand.Add(CardData{std::move(generated)});
  }
  return TaskStatus::COMPLETE;
}
}
