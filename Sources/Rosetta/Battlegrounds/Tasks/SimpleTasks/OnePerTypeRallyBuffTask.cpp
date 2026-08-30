#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
namespace {
bool SameEntity(const Minion& left, const Minion& right) {
  if (left.GetIndex() >= 0 && right.GetIndex() >= 0)
    return left.GetIndex() == right.GetIndex();
  if (left.GetPoolIndex() >= 0 && right.GetPoolIndex() >= 0)
    return left.GetPoolIndex() == right.GetPoolIndex();
  return left.GetCardID() == right.GetCardID() &&
         left.GetZonePosition() == right.GetZonePosition();
}
}

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
      selected->SetAttack(selected->GetAttack() + m_attack);
      selected->SetHealth(selected->GetHealth() + m_health);
      if (player.isInCombat) {
        player.recruitField.ForEachAlive([&](MinionData& data) {
          if (SameEntity(data.value(), *selected)) {
            data.value().SetAttack(data.value().GetAttack() + m_attack);
            data.value().SetHealth(data.value().GetHealth() + m_health);
          }
        });
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
