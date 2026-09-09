#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus OnePerTypeRallyBuffTask::Run(Player& player, Minion& source) {
  auto& field = player.GetField();
  bool lightfangPortrait = false;
  for (const auto& trinket : player.season14.trinkets) {
    if (!trinket.active || trinket.remainingUses == 0) continue;
    const auto behavior = FindTrinketBehavior(
        Cards::FindCardByDbfID(trinket.dbfID).id);
    if (behavior.portraitEffect == PortraitEffect::LIGHTFANG_ALL_TYPES) {
      lightfangPortrait = true;
      break;
    }
  }
  const bool sourceIsLightfang = source.GetCardID() == "BGS_009" ||
                                 source.GetCardID() == "TB_BaconUps_082";
  for (int repeat = 0; repeat < m_repeats; ++repeat) {
    for (const Race race : RACES_IN_BATTLEGROUNDS) {
      std::vector<Minion*> candidates;
      if (lightfangPortrait && sourceIsLightfang)
        candidates.push_back(&source);
      field.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &source && data.value().HasRace(race))
          candidates.push_back(&data.value());
      });
      if (candidates.empty())
        continue;
      auto* selected = candidates[effolkronium::random_thread_local::get<std::size_t>(
          0, candidates.size() - 1)];
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
