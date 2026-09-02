#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyAdjacentEnemyDamageTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>

using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyAdjacentEnemyDamageTask::Run(Player& player, Minion& source,
                                             Minion& target) {
  if (source.GetAttack() <= 0 || target.IsDestroyed()) return TaskStatus::STOP;
  Player& opponent = player.getOpponentPlayerCallback(player);
  const int targetPosition = target.GetZonePosition();
  std::vector<Minion*> adjacent;
  opponent.GetField().ForEachAlive([&](MinionData& data) {
    const int position = data.value().GetZonePosition();
    if (position == targetPosition - 1 || position == targetPosition + 1)
      adjacent.push_back(&data.value());
  });
  target.TakeDamage(source.GetAttack());
  if (m_bothAdjacent) {
    for (auto* adjacentTarget : adjacent) adjacentTarget->TakeDamage(source.GetAttack());
  } else if (!adjacent.empty()) {
    adjacent[Random::get<std::size_t>(0, adjacent.size() - 1)]->TakeDamage(source.GetAttack());
  }
  return TaskStatus::COMPLETE;
}
} // namespace RosettaStone::Battlegrounds::SimpleTasks
