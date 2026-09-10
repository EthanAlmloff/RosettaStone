#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ProtossBehaviorTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
namespace {
// Warp Gate's complete pinned Protoss pool, including its generated Zealot
// and Interceptor entities. Their card mechanics are supplied by the normal
// Minion/Battle runtime; this helper owns eligibility and hand-cap handling.
constexpr int PROTOSS[] = {113732, 113165, 113174, 113175, 113177, 113203,
                           113735, 113738, 113739, 113733};
void AddProtoss(Player& player, int count) {
  if (player.hand.IsFull()) return;
  std::vector<Card> pool;
  for (const auto dbf : PROTOSS) {
    const auto card = Cards::FindCardByDbfID(dbf);
    // CardDefs carries the explicit generated-token lifecycle tag. Reuse the
    // same allowlist for Warp Gate and Mothership rewards.
    if (IsExecutableWarpGateProtossDbfID(dbf) && card.hasBehavior &&
        card.normalDbfID == 0)
      pool.push_back(card);
  }
  Random::shuffle(pool.begin(), pool.end());
  for (int i = 0; i < count && i < static_cast<int>(pool.size()) && !player.hand.IsFull(); ++i)
    player.hand.Add(CardData{Minion{pool[static_cast<std::size_t>(i)]}});
}
void DamageEnemyCharacters(Player& player, int damage) {
  auto& enemy = player.getOpponentPlayerCallback(player);
  enemy.hero.TakeDamage(enemy, damage, HeroDamageSource::COMBAT_OPPONENT);
  enemy.GetField().ForEachAlive([damage](MinionData& data) {
    data.value().TakeDamage(damage);
  });
}
void DestroyRandomEnemyMinion(Player& player) {
  auto& enemy = player.getOpponentPlayerCallback(player);
  std::vector<Minion*> targets;
  enemy.GetField().ForEachAlive([&targets](MinionData& data) {
    targets.push_back(&data.value());
  });
  if (targets.empty()) return;
  auto& target = *targets[Random::get<std::size_t>(0, targets.size() - 1)];
  target.DestroyImmediately();
}
void MergeTemplars(Player& player, Minion& source) {
  if (source.GetCardID() != "SC_752" && source.GetCardID() != "SC_765") return;
  Minion* other = nullptr;
  player.recruitField.ForEachAlive([&](MinionData& data) {
    if (&data.value() != &source &&
        (data.value().GetCardID() == "SC_752" ||
         data.value().GetCardID() == "SC_765")) other = &data.value();
  });
  if (other == nullptr) return;
  const auto archon = Cards::FindCardByDbfID(113202);
  if (archon.dbfID == 0 || !source.TransformTo(archon)) return;
  const auto poolIndex = other->GetPoolIndex();
  player.recruitField.Remove(*other);
  if (player.returnMinionCallback) player.returnMinionCallback(poolIndex);
}
}
void ProtossBehaviorTask::AddProtossToHand(Player& player, int count) {
  AddProtoss(player, count);
}
TaskStatus ProtossBehaviorTask::Run(Player& player, Minion& source) {
  switch (m_effect) {
    case Effect::MOTHERSHIP_REWARD: AddProtoss(player, 2); return TaskStatus::COMPLETE;
    case Effect::COLOSSUS_DAMAGE: DamageEnemyCharacters(player, 1); return TaskStatus::COMPLETE;
    case Effect::HIGH_TEMPLAR_DAMAGE: DamageEnemyCharacters(player, 2); return TaskStatus::COMPLETE;
    case Effect::IMMORTAL_DOUBLE:
      if (player.remainCoin < 4) return TaskStatus::STOP;
      player.remainCoin -= 4; player.RecordGoldSpent(4);
      source.SetAttack(source.GetAttack() * 2);
      source.SetHealth(source.GetHealth() * 2);
      return TaskStatus::COMPLETE;
    case Effect::SENTRY_DISCOUNT: ++player.season14.protossCostReduction; return TaskStatus::COMPLETE;
    case Effect::VOID_RAY_BONUS:
      if (source.GetGameTag(GameTag::COST) == 0) { source.SetAttack(source.GetAttack()+2); source.SetHealth(source.GetHealth()+2); }
      return TaskStatus::COMPLETE;
    case Effect::CARRIER_END_TURN:
      player.season14.carrierInterceptors += 4;
      return TaskStatus::COMPLETE;
    case Effect::ARCHON_END_TURN: {
      auto& enemy = player.getOpponentPlayerCallback(player);
      enemy.hero.TakeDamage(enemy, 8, HeroDamageSource::COMBAT_OPPONENT);
      enemy.GetField().ForEachAlive([](MinionData& data) {
        data.value().TakeDamage(2);
      });
      return TaskStatus::COMPLETE;
    }
    case Effect::DARK_TEMPLAR_DESTROY:
      DestroyRandomEnemyMinion(player);
      MergeTemplars(player, source);
      return TaskStatus::COMPLETE;
  }
  return TaskStatus::STOP;
}
TaskStatus ProtossBehaviorTask::Run(Player& player, Minion& source, Minion& target) {
  // Protoss's printed target is an enemy minion.  Recruit-field target
  // indices are friendly-only in RosettaStone, so never reinterpret a stale
  // or replayed friendly index as the enemy target.  Resolve from the
  // opponent field through the same authoritative path instead.
  (void)target;
  return Run(player, source);
}
}
