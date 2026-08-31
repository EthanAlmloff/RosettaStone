#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomEnemyDamageTask.hpp>

#include <effolkronium/random.hpp>
#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomEnemyDamageTask::Run(Player& player, Minion&) {
  if (m_damage < 0 || m_count <= 0) return TaskStatus::STOP;
  Player& opponent = player.getOpponentPlayerCallback(player);
  bool damaged = false;
  for (int shot = 0; shot < m_count; ++shot) {
    std::vector<Minion*> candidates;
    opponent.GetField().ForEachAlive([&candidates](MinionData& data) {
      candidates.push_back(&data.value());
    });
    if (candidates.empty()) break;
    Minion& target = *candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
    target.TakeDamage(m_damage);
    damaged = true;
  }
  return damaged ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus RandomEnemyDamageTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
