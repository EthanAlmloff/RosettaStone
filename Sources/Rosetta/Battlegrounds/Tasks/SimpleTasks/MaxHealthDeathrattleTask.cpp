#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MaxHealthDeathrattleTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MaxHealthDeathrattleTask::Run(Player& player, Minion& source) {
  if (m_repeats <= 0) return TaskStatus::STOP;
  Minion* target = nullptr;
  player.GetField().ForEachAlive([&](MinionData& data) {
    if (target == nullptr && &data.value() != &source) target = &data.value();
  });
  if (target == nullptr) return TaskStatus::STOP;
  target->SetHealth(target->GetHealth() + source.GetMaxHealth() * m_repeats);
  return TaskStatus::COMPLETE;
}
TaskStatus MaxHealthDeathrattleTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
