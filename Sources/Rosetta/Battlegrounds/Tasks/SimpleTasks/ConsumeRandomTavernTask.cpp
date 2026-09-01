#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeRandomTavernTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
namespace {
TaskStatus Consume(Player& player, Minion& target, int multiplier) {
  if (multiplier <= 0 || target.IsDestroyed() || !target.HasRace(Race::DEMON)) return TaskStatus::STOP;
  std::vector<int> candidates;
  player.tavern.fieldZone.ForEachAlive([&candidates](MinionData& data) {
    if (data.value().GetPoolIndex() >= 0) candidates.push_back(data.value().GetZonePosition());
  });
  if (candidates.empty()) return TaskStatus::STOP;
  const auto slot = candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
  auto consumed = player.tavern.fieldZone.Remove(player.tavern.fieldZone[static_cast<std::size_t>(slot)]);
  player.returnMinionCallback(consumed.GetPoolIndex());
  target.SetAttack(target.GetAttack() + consumed.GetAttack() * multiplier);
  target.SetHealth(target.GetHealth() + consumed.GetHealth() * multiplier);
  return TaskStatus::COMPLETE;
}
}
TaskStatus ConsumeRandomTavernTask::Run(Player& p, Minion& owner) { return Consume(p, owner, m_multiplier); }
TaskStatus ConsumeRandomTavernTask::Run(Player& p, Minion&, Minion& target) { return Consume(p, target, m_multiplier); }
}
