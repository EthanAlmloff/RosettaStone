#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
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
  // Consuming Claw is a successful-consumption observer.  Resolve it after
  // the normal devour payload, once per owned Trinket instance, so duplicate
  // copies stack and failed/full-shop attempts remain inert.
  for (const auto& trinket : player.season14.trinkets) {
    if (!trinket.active || trinket.remainingUses == 0) continue;
    const auto id = Cards::FindCardByDbfID(trinket.dbfID).id;
    const auto behavior = FindTrinketBehavior(id);
    if (behavior.effect != TrinketEffect::DEMON_CONSUME_BONUS_KEYWORDS)
      continue;
    if (consumed.HasTaunt()) target.SetTaunt(true);
    if (consumed.HasDivineShield()) target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
    if (consumed.HasReborn()) target.SetReborn(true);
    if (consumed.HasWindfury()) target.SetGameTag(GameTag::WINDFURY, 1);
    if (consumed.HasVenomous()) target.SetGameTag(GameTag::VENOMOUS, 1);
    if (consumed.HasStealth()) target.SetGameTag(GameTag::STEALTH, 1);
    target.SetAttack(target.GetAttack() + behavior.attack);
    target.SetHealth(target.GetHealth() + behavior.health);
  }
  return TaskStatus::COMPLETE;
}
}
TaskStatus ConsumeRandomTavernTask::Run(Player& p, Minion& owner) { return Consume(p, owner, m_multiplier); }
TaskStatus ConsumeRandomTavernTask::Run(Player& p, Minion&, Minion& target) { return Consume(p, target, m_multiplier); }
}
