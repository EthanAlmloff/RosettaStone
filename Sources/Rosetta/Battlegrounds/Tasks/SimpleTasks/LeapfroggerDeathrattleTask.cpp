#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeapfroggerDeathrattleTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus LeapfroggerDeathrattleTask::Run(Player& player, Minion& owner) {
  return Run(player, owner, owner);
}
TaskStatus LeapfroggerDeathrattleTask::Run(Player& player, Minion& owner, Minion&) {
  if (!player.isInCombat) return TaskStatus::STOP;
  std::vector<Minion*> candidates;
  player.GetField().ForEachAlive([&](MinionData& data) {
    Minion& candidate = data.value();
    if (&candidate != &owner && candidate.HasRace(Race::BEAST)) candidates.push_back(&candidate);
  });
  if (candidates.empty()) return TaskStatus::STOP;
  const auto index = Random::get<std::size_t>(0, candidates.size() - 1);
  Minion& target = *candidates[index];
  target.SetAttack(target.GetAttack() + m_attack);
  target.SetHealth(target.GetHealth() + m_health);
  owner.CopyDeathrattleTo(target);
  return TaskStatus::COMPLETE;
}
}
