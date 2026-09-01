#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnGoldenCountSelfBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnGoldenCountSelfBuffTask::Run(Player& player, Minion& source) {
  if (m_amount <= 0 || source.IsDestroyed()) return TaskStatus::STOP;
  int golden = 0;
  player.GetField().ForEachAlive([&](MinionData& data) { if (data.value().IsGolden()) ++golden; });
  if (golden == 0) return TaskStatus::COMPLETE;
  source.SetAttack(source.GetAttack() + m_amount * golden);
  source.SetHealth(source.GetHealth() + m_amount * golden);
  return TaskStatus::COMPLETE;
}
TaskStatus EndTurnGoldenCountSelfBuffTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
