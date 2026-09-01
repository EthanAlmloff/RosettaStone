#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomMagneticMechToTargetTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomMagneticMechToTargetTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
TaskStatus RandomMagneticMechToTargetTask::Run(Player& player, Minion&, Minion& target) {
  if (target.IsDestroyed()) return TaskStatus::STOP;
  std::vector<const Card*> candidates;
  for (const auto& card : Cards::GetAllCards()) {
    if (!card.isBattlegroundsPoolMinion || !card.hasBehavior ||
        card.GetCardType() != CardType::MINION || card.normalDbfID != 0 ||
        !card.HasRace(Race::MECHANICAL) ||
        !card.gameTags.contains(GameTag::MAGNETIC) ||
        card.gameTags.at(GameTag::MAGNETIC) == 0) continue;
    candidates.push_back(&card);
  }
  if (candidates.empty()) return TaskStatus::STOP;
  Minion attachment{*candidates[Random::get<std::size_t>(0, candidates.size() - 1)]};
  attachment.MagnetizeOnto(target);
  return TaskStatus::COMPLETE;
}
}
