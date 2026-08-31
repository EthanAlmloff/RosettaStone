#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DarkGiftRandomPoolTask.hpp>
#include <effolkronium/random.hpp>
#include <unordered_set>
#include <vector>

using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DarkGiftRandomPoolTask::Run(Player& player, Minion&) {
  if (player.hand.IsFull()) return TaskStatus::STOP;
  std::vector<const Card*> candidates;
  Race race = Race::ALL;
  if (m_pool == Pool::MOST_COMMON_RACE_MINION) {
    static constexpr Race races[] = {Race::BLOODELF, Race::DRAENEI, Race::DWARF, Race::GNOME, Race::GOBLIN, Race::HUMAN, Race::NIGHTELF, Race::ORC, Race::TAUREN, Race::TROLL, Race::UNDEAD, Race::MURLOC, Race::DEMON, Race::MECHANICAL, Race::ELEMENTAL, Race::BEAST, Race::PIRATE, Race::DRAGON, Race::QUILBOAR, Race::NAGA, Race::CELESTIAL};
    int best = 0;
    for (const auto candidate : races) {
      int count = 0;
      player.recruitField.ForEachAlive([&](const MinionData& data) { if (data.value().HasRace(candidate)) ++count; });
      if (count > best) { best = count; race = candidate; }
    }
    // Charisma has no resolvable "most common type" on an empty board; it
    // must not silently widen its pool to every minion type.
    if (best == 0) return TaskStatus::STOP;
  }
  std::unordered_set<std::string_view> seen;
  for (const auto& card : Cards::GetAllCards()) {
    const bool spell = card.isBattlegroundsPoolSpell && (card.GetCardType() == CardType::SPELL || card.GetCardType() == CardType::BATTLEGROUND_SPELL);
    const bool minion = card.isBattlegroundsPoolMinion && card.GetCardType() == CardType::MINION && card.normalDbfID == 0 && (race == Race::ALL || card.HasRace(race));
    if ((m_pool == Pool::TAVERN_SPELL ? spell : minion) && seen.insert(card.id).second) candidates.push_back(&card);
  }
  if (candidates.empty()) return TaskStatus::STOP;
  const auto* card = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
  if (m_pool == Pool::TAVERN_SPELL) player.hand.Add(CardData{Spell(*card)});
  else { Minion generated{*card}; player.ApplyFreshMinionModifiers(generated); player.hand.Add(CardData{std::move(generated)}); }
  return TaskStatus::COMPLETE;
}
TaskStatus DarkGiftRandomPoolTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
