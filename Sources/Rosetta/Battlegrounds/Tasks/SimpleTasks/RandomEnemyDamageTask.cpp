#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

#include <effolkronium/random.hpp>
#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomEnemyDamageTask::Run(Player& player, Minion& source) {
  if (m_damage < 0 || m_count <= 0) return TaskStatus::STOP;
  int damage = m_damage;
  // Kaboom Bot Portrait modifies only Kaboom Bot deathrattles, including
  // golden Bots. Resolve it here so other random-damage tasks are unchanged.
  if (source.GetCardID() == "BG_BOT_606" ||
      source.GetCardID() == "TB_BaconUps_028") {
    for (const auto& trinket : player.season14.trinkets) {
      if (!trinket.active || trinket.remainingUses == 0) continue;
      const auto behavior = FindTrinketBehavior(
          Cards::FindCardByDbfID(trinket.dbfID).id);
      if (behavior.portraitEffect ==
          PortraitEffect::KABOOM_BOT_DEATHRATTLE_DAMAGE) {
        damage += 10;
        break;
      }
    }
  }
  // Keep the effective value in the task-shaped name used by the shared
  // damage contract while leaving the constructor payload immutable.
  const int m_damage = damage;
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
