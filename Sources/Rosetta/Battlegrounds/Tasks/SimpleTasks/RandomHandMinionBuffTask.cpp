#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomHandMinionBuffTask.hpp>

#include <effolkronium/random.hpp>

#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomHandMinionBuffTask::Run(Player& player, Minion&) {
  std::vector<Minion*> candidates;
  player.hand.ForEach([&candidates](std::optional<CardData>& card) {
    if (card.has_value() && std::holds_alternative<Minion>(card.value()))
      candidates.push_back(&std::get<Minion>(card.value()));
  });
  if (candidates.empty()) return TaskStatus::STOP;
  const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
  Minion& target = *candidates[index];
  target.SetAttack(target.GetAttack() + m_attack);
  target.SetHealth(target.GetHealth() + m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus RandomHandMinionBuffTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
