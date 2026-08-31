#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus OnePerTypeRallyBuffTask::Run(Player& player, Minion&) {
  auto& field = player.GetField();
  for (int repeat = 0; repeat < m_repeats; ++repeat) {
    for (const Race race : RACES_IN_BATTLEGROUNDS) {
      Minion* selected = nullptr;
      field.ForEachAlive([&](MinionData& data) {
        if (selected == nullptr && data.value().HasRace(race))
          selected = &data.value();
      });
      if (selected == nullptr)
        continue;
      if (player.isInCombat)
        selected->ApplyCombatPersistentStats(m_attack, m_health);
      else {
        selected->SetAttack(selected->GetAttack() + m_attack);
        selected->SetHealth(selected->GetHealth() + m_health);
      }
    }
  }
  return TaskStatus::COMPLETE;
}

TaskStatus OnePerTypeRallyBuffTask::Run(Player& player, Minion& source,
                                        Minion&) {
  return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
