// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <stdexcept>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
namespace
{
struct AttackingStateGuard
{
    Minion& minion;
    const bool previous;

    explicit AttackingStateGuard(Minion& value)
        : minion(value), previous(value.IsAttacking())
    {
        minion.SetAttacking(true);
    }

    ~AttackingStateGuard() { minion.SetAttacking(previous); }
};

bool HasAttackableTarget(const FieldZone& field)
{
    bool found = false;
    field.ForEachAlive([&found](const MinionData& minion) {
        if (!minion.value().HasStealth())
        {
            found = true;
        }
    });
    return found;
}

void ConsumeRebornInRecruitField(Player& owner,
                                 const Minion& consumedCombatMinion)
{
    const int entityIndex = consumedCombatMinion.GetIndex();
    const int zonePosition = consumedCombatMinion.GetZonePosition();
    const std::string cardID(consumedCombatMinion.GetCardID());

    owner.recruitField.ForEachAlive(
        [entityIndex, zonePosition, &cardID](MinionData& minion) {
            Minion& source = minion.value();
            const bool sameEntity =
                entityIndex >= 0 ? source.GetIndex() == entityIndex
                                 : (source.GetZonePosition() == zonePosition &&
                                    source.GetCardID() == cardID);
            if (sameEntity && source.HasReborn())
            {
                // Combat operates on a copy of recruitField.  Persist the
                // one-shot keyword consumption back to the source instance,
                // but only when that source really owned Reborn; a combat
                // task may grant Reborn to the temporary copy alone.
                source.SetReborn(false);
            }
        });
}

void ApplyPermanentAvengeBonus(Player& owner, FieldZone& combatField,
                              int attack, int health)
{
    if (attack == 0 && health == 0)
    {
        return;
    }

    std::vector<Minion> combatSources;
    combatSources.reserve(MAX_FIELD_SIZE);
    combatField.ForEachAlive([&combatSources, attack, health](MinionData& data) {
        Minion& combatMinion = data.value();
        combatSources.push_back(combatMinion);
        // The bonus is permanent and also affects the current combat copy.
        combatMinion.SetAttack(combatMinion.GetAttack() + attack);
        combatMinion.SetHealth(combatMinion.GetHealth() + health);
    });

    // Combat uses copied entities.  Commit only the Avenge delta to matching
    // recruit-phase entities; combat damage and temporary effects are not
    // persistent and must not be copied wholesale.
    owner.recruitField.ForEachAlive(
        [&combatSources, attack, health](MinionData& data) {
            Minion& recruitMinion = data.value();
            const auto source = std::find_if(
                combatSources.begin(), combatSources.end(),
                [&recruitMinion](const Minion& combatMinion) {
                    return combatMinion.IsSameInstance(recruitMinion);
                });
            if (source != combatSources.end())
            {
                recruitMinion.SetAttack(recruitMinion.GetAttack() + attack);
                recruitMinion.SetHealth(recruitMinion.GetHealth() + health);
            }
        });
}
}  // namespace

Battle::Battle(Player& player1, Player& player2)
    : m_player1(player1),
      m_player2(player2),
      m_p1Field(m_player1.battleField),
      m_p2Field(m_player2.battleField)
{
    m_player1.season14.ClearCombatExactCopySnapshots();
    m_player2.season14.ClearCombatExactCopySnapshots();
    m_player1.battleField = m_player1.recruitField;
    m_player2.battleField = m_player2.recruitField;
    const auto summonHandSnapshot = [](Player& owner, FieldZone& field) {
        auto snapshots = owner.season14.TakeCombatHandSummons();
        for (auto& [snapshot, count] : snapshots) {
            for (int i = 0; i < count && !field.IsFull(); ++i) {
                Minion copy{snapshot};
                // A combat-only copy is a fresh entity.  Preserve the full
                // card instance (stats, keywords, enchantments and tasks),
                // but give it the owning player's callback and a new entity
                // index so trigger/source identity cannot alias the hand
                // instance or another summoned copy.
                owner.ApplyFreshMinionModifiers(copy);
                copy.getPlayerCallback = [&owner]() -> Player& { return owner; };
                if (owner.getNextCardIndexCallback)
                    copy.SetIndex(owner.getNextCardIndexCallback());
                field.Add(copy);
            }
        }
    };
    summonHandSnapshot(m_player1, m_p1Field);
    summonHandSnapshot(m_player2, m_p2Field);
    const auto applyHighestHandAttack = [](Player& owner, FieldZone& field) {
        int highest = 0;
        owner.hand.ForEach([&](const std::optional<CardData>& entry) {
            if (std::holds_alternative<Minion>(*entry)) highest = std::max(highest, std::get<Minion>(*entry).GetAttack());
        });
        field.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG34_142" || minion.GetCardID() == "BG34_142_G")
                minion.SetAttack(minion.GetAttack() + highest * (minion.IsGolden() ? 2 : 1));
        });
    };
    applyHighestHandAttack(m_player1, m_p1Field);
    applyHighestHandAttack(m_player2, m_p2Field);
    const auto apply = [](Player& owner, FieldZone& field) {
        const auto doubles = owner.season14.TakeCombatStartLeftmostAttackDoubles();
        for (std::size_t i = 0; i < doubles; ++i)
        {
            bool applied = false;
            field.ForEachAlive([&applied](MinionData& data) {
                // The field iteration is ordered; only the first occupied
                // slot is affected.  Mutate the combat copy only.
                if (!applied)
                {
                    data.value().SetAttack(data.value().GetAttack() * 2);
                    applied = true;
                }
            });
        }
    };
    // Resolve player-owned spell effects in seat order, before any combat
    // triggers run.  The order is explicit so seeded RNG and cross-player
    // nearest-stat interactions remain replay-stable.
    apply(m_player1, m_p1Field);
    apply(m_player2, m_p2Field);
    const auto copyNearest = [](Player& owner, FieldZone& ownField,
                                const FieldZone& enemyField) {
        const auto copies = owner.season14.TakeCombatStartNearestStats();
        for (std::size_t i = 0; i < copies; ++i)
        {
            Minion* leftmost = nullptr;
            ownField.ForEachAlive([&leftmost](MinionData& data) {
                if (leftmost == nullptr)
                    leftmost = &data.value();
            });
            if (leftmost == nullptr)
                continue;
            const Minion* nearest = nullptr;
            int bestDistance = std::numeric_limits<int>::max();
            enemyField.ForEachAlive([&](const MinionData& data) {
                const Minion& candidate = data.value();
                const int distance = std::abs(
                    candidate.GetZonePosition() - leftmost->GetZonePosition());
                if (nearest == nullptr || distance < bestDistance ||
                    (distance == bestDistance &&
                     candidate.GetZonePosition() < nearest->GetZonePosition()))
                {
                    nearest = &candidate;
                    bestDistance = distance;
                }
            });
            if (nearest != nullptr)
            {
                leftmost->SetAttack(leftmost->GetAttack() + nearest->GetAttack());
                leftmost->SetHealth(leftmost->GetHealth() + nearest->GetHealth());
            }
        }
    };
    copyNearest(m_player1, m_p1Field, m_p2Field);
    copyNearest(m_player2, m_p2Field, m_p1Field);
    const auto setRandomHealth = [](Player& owner, FieldZone& enemyField) {
        const auto count = owner.season14.TakeCombatStartRandomEnemySetHealth();
        for (std::size_t i = 0; i < count; ++i)
        {
            std::vector<Minion*> candidates;
            enemyField.ForEachAlive([&candidates](MinionData& data) {
                candidates.push_back(&data.value());
            });
            if (!candidates.empty())
                candidates[static_cast<std::size_t>(Random::get<int>(
                    0, static_cast<int>(candidates.size() - 1)))]
                    ->SetHealth(1);
        }
    };
    setRandomHealth(m_player1, m_p2Field);
    setRandomHealth(m_player2, m_p1Field);
    const auto summonBeetles = [](Player& owner, FieldZone& field) {
        const auto casts = owner.season14.TakeCombatStartBeetles();
        const Card beetleCard = Cards::FindCardByID("BG28_603t");
        for (std::size_t cast = 0; cast < casts && !field.IsFull(); ++cast)
            for (int i = 0; i < 2 && !field.IsFull(); ++i)
            {
                Minion beetle{ beetleCard };
                owner.ApplyFreshMinionModifiers(beetle);
                field.Add(beetle);
            }
    };
    summonBeetles(m_player1, m_p1Field);
    summonBeetles(m_player2, m_p2Field);
}

void Battle::CommitPersistentState()
{
    const auto commit = [](Player& owner, const FieldZone& combatField) {
        owner.recruitField.ForEachAlive([&](MinionData& recruitData) {
            Minion& recruit = recruitData.value();
            const Minion* combat = nullptr;
            combatField.ForEachAlive([&](const MinionData& combatData) {
                if (combat == nullptr &&
                    combatData.value().IsSameInstance(recruit))
                    combat = &combatData.value();
            });
            if (combat != nullptr)
                recruit.ReconcileCombatPersistentState(*combat);
        });
    };
    commit(m_player1, m_p1Field);
    commit(m_player2, m_p2Field);
}

void Battle::Initialize()
{
    m_p1Field.ForEachAlive([](MinionData& data) { data.value().ResetAvengeProgress(); });
    m_p2Field.ForEachAlive([](MinionData& data) { data.value().ResetAvengeProgress(); });
    m_player1.season14.TakeCombatAvengeCards();
    m_player2.season14.TakeCombatAvengeCards();
    m_player1.season14.BeginCombatBatch4();
    m_player2.season14.BeginCombatBatch4();

    // Determine the player attacks first
    // NOTE: The player with the greater number of minions attacks first.
    // If the number of minions is equal for both players, one of the players
    // is randomly selected to attack first.
    const int p1NumMinions = m_p1Field.GetCount();
    const int p2NumMinions = m_p2Field.GetCount();

    if (p1NumMinions > p2NumMinions)
    {
        m_turn = Turn::PLAYER1;
    }
    else if (p1NumMinions < p2NumMinions)
    {
        m_turn = Turn::PLAYER2;
    }
    else
    {
        m_turn = static_cast<Turn>(Random::get<int>(0, 1));
    }

    m_p1NextAttackerIdx = 0;
    m_p2NextAttackerIdx = 0;
    m_p1PendingAttacks = 0;
    m_p2PendingAttacks = 0;

    // Dark Gift combat-start multipliers are attached to the copied combat
    // minions, so they are applied exactly once per combat before any
    // START_OF_COMBAT task observes stats.
    m_p1Field.ForEach([](MinionData& minion) {
        minion.value().ResetFrenzyUses();
        minion.value().ApplyStartCombatStatMultipliers();
    });
    m_p2Field.ForEach([](MinionData& minion) {
        minion.value().ResetFrenzyUses();
        minion.value().ApplyStartCombatStatMultipliers();
    });

    // Jaws of Death is a Dark Gift state on the copied combat minion.  It
    // triggers that minion's own Deathrattle once, before ordinary
    // START_OF_COMBAT tasks, matching the card's timing and preserving the
    // existing seat-order sequencing.
    const auto triggerGiftDeathrattles = [](FieldZone& field, Player& owner) {
        field.ForEach([&owner](MinionData& data) {
            auto& minion = data.value();
            while (minion.HasStartCombatDeathrattleTrigger() &&
                   minion.HasDeathrattle()) {
                minion.ActivateTask(PowerType::DEATHRATTLE, owner);
                minion.ConsumeStartCombatDeathrattleTrigger();
            }
        });
    };
    if (m_turn == Turn::PLAYER1)
    {
        triggerGiftDeathrattles(m_p1Field, m_player1);
        triggerGiftDeathrattles(m_p2Field, m_player2);
    }
    else
    {
        triggerGiftDeathrattles(m_p2Field, m_player2);
        triggerGiftDeathrattles(m_p1Field, m_player1);
    }

    const auto applyGiftLeftAttack = [](FieldZone& field) {
        std::vector<Minion*> alive;
        field.ForEachAlive([&alive](MinionData& data) {
            alive.push_back(&data.value());
        });
        for (std::size_t i = 1; i < alive.size(); ++i)
            while (alive[i]->HasStartCombatLeftAttack())
                alive[i]->ApplyStartCombatLeftAttack(*alive[i - 1]);
    };
    applyGiftLeftAttack(m_p1Field);
    applyGiftLeftAttack(m_p2Field);

    if (m_turn == Turn::PLAYER1)
    {
        m_p1Field.ForEach([this](MinionData& minion) {
            minion.value().ActivateTask(PowerType::START_OF_COMBAT, m_player1);
        });
        m_p2Field.ForEach([this](MinionData& minion) {
            minion.value().ActivateTask(PowerType::START_OF_COMBAT, m_player2);
        });
    }
    else
    {
        m_p2Field.ForEach([this](MinionData& minion) {
            minion.value().ActivateTask(PowerType::START_OF_COMBAT, m_player2);
        });
        m_p1Field.ForEach([this](MinionData& minion) {
            minion.value().ActivateTask(PowerType::START_OF_COMBAT, m_player1);
        });
    }

    ProcessDestroy(true);
}

CombatResult Battle::Run()
{
    Initialize();

    bool prevAttackSuccess = false;
    Turn turnStart = Turn::DONE;

    while (!IsDone())
    {
        // A Windfury/Mega-Windfury sequence stays on the same combat turn.
        // Start-of-turn triggers therefore run once when a side receives the
        // turn, not once for every repeated attack in that sequence.
        if (m_turn != turnStart)
        {
            if (m_turn == Turn::PLAYER1)
            {
                m_p1Field.ForEachAlive([this](MinionData& owner) {
                    m_p1Field.ForEachAlive([&owner](MinionData& minion) {
                        owner.value().ActivateTrigger(TriggerType::TURN_START,
                                                      minion.value());
                    });
                });

                m_p2Field.ForEachAlive([this](MinionData& owner) {
                    m_p2Field.ForEachAlive([&owner](MinionData& minion) {
                        owner.value().ActivateTrigger(TriggerType::TURN_START,
                                                      minion.value());
                    });
                });
            }
            else
            {
                m_p2Field.ForEachAlive([this](MinionData& owner) {
                    m_p2Field.ForEachAlive([&owner](MinionData& minion) {
                        owner.value().ActivateTrigger(TriggerType::TURN_START,
                                                      minion.value());
                    });
                });

                m_p1Field.ForEachAlive([this](MinionData& owner) {
                    m_p1Field.ForEachAlive([&owner](MinionData& minion) {
                        owner.value().ActivateTrigger(TriggerType::TURN_START,
                                                      minion.value());
                    });
                });
            }
            turnStart = m_turn;
        }

        const bool curAttackSuccess = Attack();
        if (!prevAttackSuccess && !curAttackSuccess)
        {
            m_turn = Turn::DONE;
            break;
        }

        prevAttackSuccess = curAttackSuccess;
    }

    ProcessResult();

    const int damage = CalculateDamage();
    if (m_result == BattleResult::PLAYER1_WIN)
    {
        m_player2.hero.TakeDamage(m_player2, damage,
                                   HeroDamageSource::COMBAT_OPPONENT);
    }
    else if (m_result == BattleResult::PLAYER2_WIN)
    {
        m_player1.hero.TakeDamage(m_player1, damage,
                                   HeroDamageSource::COMBAT_OPPONENT);
    }
    return { m_result, damage, static_cast<int>(m_player1.idx),
             static_cast<int>(m_player2.idx) };
}

bool Battle::Attack()
{
    const int attackerIdx = FindAttacker();
    // No minions that can attack, switch players
    if (attackerIdx == -1)
    {
        m_turn = (m_turn == Turn::PLAYER1) ? Turn::PLAYER2 : Turn::PLAYER1;
        return false;
    }

    Minion& attacker = (m_turn == Turn::PLAYER1) ? m_p1Field[attackerIdx]
                                                 : m_p2Field[attackerIdx];

    FieldZone& defendingField =
        (m_turn == Turn::PLAYER1) ? m_p2Field : m_p1Field;
    if (!HasAttackableTarget(defendingField))
    {
        // Stealthed minions cannot be selected by a Battlegrounds attack. If
        // every opposing minion is hidden, this side simply has no legal
        // attack target until stealth is removed.
        if (m_turn == Turn::PLAYER1)
        {
            m_p1PendingAttacks = 0;
        }
        else
        {
            m_p2PendingAttacks = 0;
        }
        m_turn = (m_turn == Turn::PLAYER1) ? Turn::PLAYER2 : Turn::PLAYER1;
        return false;
    }

    int& pendingAttacks =
        (m_turn == Turn::PLAYER1) ? m_p1PendingAttacks : m_p2PendingAttacks;
    if (pendingAttacks == 0)
    {
        // Store the current attack as well as any repeats.  A zero value is
        // reserved for "no Windfury sequence is in progress"; this keeps the
        // final attack of a two-hit sequence from starting a new sequence.
        pendingAttacks = attacker.GetAttackCount();
    }
    const bool shouldRepeat = pendingAttacks > 1;
    --pendingAttacks;

    // Keep a stable fallback for hand-built tests whose minions do not have
    // an assigned entity index.  Normal game minions use GetIndex().
    const int attackerEntityIndex = attacker.GetIndex();
    const int attackerZonePosition = attacker.GetZonePosition();
    const std::string attackerCardID(attacker.GetCardID());
    const bool attackerHadReborn = attacker.HasReborn();

    Minion& target = GetProperTarget(attacker);
    FieldZone& attackerField =
        (m_turn == Turn::PLAYER1) ? m_p1Field : m_p2Field;

    // Rally resolves at attack declaration, after the legal target is known
    // but before combat damage. Snapshot no references here: Rally tasks may
    // mutate either field and are responsible for their own stable lookups.
    attackerField.ForEachAlive([&](MinionData& minionData) {
        minionData.value().ActivateRally(
            (m_turn == Turn::PLAYER1) ? m_player1 : m_player2, attacker,
            target);
    });
    // Jailbird Juggernaut's Rally queues a Blood Gem Golem to attack the
    // already-selected target first. Resolve it here, after all Rally tasks
    // have run, so inserting the Golem cannot invalidate the active attacker
    // or target references during task dispatch.
    auto& attackerOwner = (m_turn == Turn::PLAYER1) ? m_player1 : m_player2;
    for (const auto& pending : attackerOwner.season14.TakeBloodGemGolemAttacks()) {
        const Card golemCard = Cards::FindCardByID("BG30_MagicItem_442t");
        if (!golemCard.id.empty() && !attackerField.IsFull()) {
            Minion golem{golemCard};
            golem.SetAttack(pending.attack);
            golem.SetHealth(pending.health);
            attackerOwner.ApplyFreshMinionModifiers(golem);
            golem.getPlayerCallback = [&attackerOwner]() -> Player& { return attackerOwner; };
            if (attackerOwner.getNextCardIndexCallback)
                golem.SetIndex(attackerOwner.getNextCardIndexCallback());
            attackerField.Add(golem, attackerField.GetCount());
            Minion& summoned = attackerField[attackerField.GetCount() - 1];
            attackerField.ForEachAlive([&summoned](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, summoned);
            });
            Minion* queuedTarget = nullptr;
            defendingField.ForEachAlive([&](MinionData& data) {
                if (data.value().GetIndex() == pending.targetEntityID)
                    queuedTarget = &data.value();
            });
            if (queuedTarget != nullptr) {
                {
                    AttackingStateGuard attacking(summoned);
                    queuedTarget->TakeDamage(summoned);
                    summoned.TakeDamage(*queuedTarget);
                }
                ProcessDestroy(false);
            }
        }
    }
    // Rally insertion may compact the attacker's field. Reacquire the source
    // by its stable entity ID before the ordinary attack exchange.
    Minion* currentAttacker = nullptr;
    attackerField.ForEachAlive([&](MinionData& data) {
        if (data.value().GetIndex() == attackerEntityIndex)
            currentAttacker = &data.value();
    });
    if (currentAttacker == nullptr) return false;
    Minion& attackerAfterRally = *currentAttacker;
    // The first attacker may have killed the original target; choose the
    // ordinary attack target only after that exchange and cleanup.
    Minion& nextTarget = GetProperTarget(attackerAfterRally);
    {
        AttackingStateGuard attacking(attackerAfterRally);
        nextTarget.TakeDamage(attackerAfterRally);
        attackerAfterRally.TakeDamage(nextTarget);
    }
    const bool targetWasDestroyed = nextTarget.IsDestroyed();

    ProcessDestroy(false);

    // A Reborn copy has already spent its attack.  Do not mistake it for the
    // original Windfury attacker when a minion dies in combat or a death
    // trigger removes it before ProcessDestroy completes.
    bool attackerSurvived = false;
    int attackerPositionAfterCleanup = -1;
    attackerField.ForEachAlive([&](MinionData& minionData) {
        const Minion& candidate = minionData.value();
        const bool sameEntity =
            attackerEntityIndex >= 0
                ? candidate.GetIndex() == attackerEntityIndex
                : (candidate.GetZonePosition() == attackerZonePosition &&
                   candidate.GetCardID() == attackerCardID);
        if (sameEntity && (!attackerHadReborn || candidate.HasReborn()))
        {
            attackerSurvived = true;
            attackerPositionAfterCleanup = candidate.GetZonePosition();
        }
    });

    // Glory of Combat is a player-owned passive.  Apply it only after the
    // combat exchange has confirmed that this attacker's damage destroyed an
    // enemy and that the same minion survived cleanup.  Looking the attacker
    // up by stable identity avoids touching a reference invalidated by zone
    // compaction or deathrattle processing.
    if (targetWasDestroyed && attackerSurvived)
    {
        const auto bonus =
            (m_turn == Turn::PLAYER1)
                ? m_player1.season14.HeroPowerBatch3CombatKillAttackBonus()
                : m_player2.season14.HeroPowerBatch3CombatKillAttackBonus();
        if (bonus > 0)
        {
            attackerField.ForEachAlive([&](MinionData& minionData) {
                Minion& candidate = minionData.value();
                const bool sameEntity =
                    attackerEntityIndex >= 0
                        ? candidate.GetIndex() == attackerEntityIndex
                        : (candidate.GetZonePosition() ==
                               attackerZonePosition &&
                           candidate.GetCardID() == attackerCardID);
                if (sameEntity)
                {
                    candidate.SetAttack(candidate.GetAttack() + bonus);
                }
            });
        }
    }

    if (shouldRepeat && attackerSurvived)
    {
        if (m_turn == Turn::PLAYER1)
        {
            m_p1NextAttackerIdx = attackerPositionAfterCleanup;
        }
        else
        {
            m_p2NextAttackerIdx = attackerPositionAfterCleanup;
        }

        return true;
    }

    pendingAttacks = 0;

    m_turn = (m_turn == Turn::PLAYER1) ? Turn::PLAYER2 : Turn::PLAYER1;
    return true;
}

int Battle::FindAttacker()
{
    FieldZone& fieldZone = (m_turn == Turn::PLAYER1) ? m_p1Field : m_p2Field;
    if (fieldZone.IsEmpty())
    {
        return -1;
    }

    int nextAttackerIdx =
        (m_turn == Turn::PLAYER1) ? m_p1NextAttackerIdx : m_p2NextAttackerIdx;

    // Deathrattles and summons can change the field while an attack is being
    // resolved.  Keep the cursor inside the compacted zone before indexing;
    // this also protects hand-built battles that remove a minion directly.
    nextAttackerIdx %= fieldZone.GetCount();
    if (nextAttackerIdx < 0)
    {
        nextAttackerIdx += fieldZone.GetCount();
    }

    for (int i = 0; i < fieldZone.GetCount(); ++i)
    {
        if (fieldZone[nextAttackerIdx].GetAttack() > 0)
        {
            return nextAttackerIdx;
        }

        ++nextAttackerIdx;
        if (nextAttackerIdx == fieldZone.GetCount())
        {
            nextAttackerIdx = 0;
        }
    }

    return -1;
}

Minion& Battle::GetProperTarget([[maybe_unused]] Minion& attacker)
{
    auto& minions = (m_turn == Turn::PLAYER1) ? m_p2Field : m_p1Field;

    std::vector<std::size_t> attackableMinions;
    std::vector<std::size_t> tauntMinions;
    attackableMinions.reserve(MAX_FIELD_SIZE);
    tauntMinions.reserve(MAX_FIELD_SIZE);

    minions.ForEachAlive([&attackableMinions,
                          &tauntMinions](const MinionData& minion) {
        const Minion& target = minion.value();
        if (target.HasStealth())
        {
            return;
        }

        const auto index = static_cast<std::size_t>(target.GetZonePosition());
        attackableMinions.emplace_back(index);
        if (target.HasTaunt())
        {
            tauntMinions.emplace_back(index);
        }
    });

    if (!tauntMinions.empty())
    {
        const auto idx = Random::get<std::size_t>(0, tauntMinions.size() - 1);
        return minions[tauntMinions[idx]];
    }

    if (attackableMinions.empty())
    {
        throw std::logic_error("No non-stealthed Battlegrounds target");
    }

    const auto idx = Random::get<std::size_t>(0, attackableMinions.size() - 1);
    return minions[attackableMinions[idx]];
}

void Battle::ProcessDestroy(bool beforeAttack)
{
    std::vector<std::tuple<int, Minion&>> deadMinions;

    if (m_turn == Turn::PLAYER1)
    {
        m_p2Field.ForEach([&deadMinions](MinionData& minion) {
            if (minion.value().IsDestroyed())
            {
                deadMinions.emplace_back(
                    std::make_tuple(2, std::ref(minion.value())));
            }
        });

        m_p1Field.ForEach([&deadMinions](MinionData& minion) {
            if (minion.value().IsDestroyed())
            {
                deadMinions.emplace_back(
                    std::make_tuple(1, std::ref(minion.value())));
            }
        });
    }
    else
    {
        m_p1Field.ForEach([&deadMinions](MinionData& minion) {
            if (minion.value().IsDestroyed())
            {
                deadMinions.emplace_back(
                    std::make_tuple(1, std::ref(minion.value())));
            }
        });

        m_p2Field.ForEach([&deadMinions](MinionData& minion) {
            if (minion.value().IsDestroyed())
            {
                deadMinions.emplace_back(
                    std::make_tuple(2, std::ref(minion.value())));
            }
        });
    }

    // A variable to check a minion at the index of next attacker is destroyed
    bool isAttackerDestroyed = false;

    for (auto& deadMinion : deadMinions)
    {
        Minion& minion = std::get<1>(deadMinion);
        Minion removedMinion;
        if (std::get<0>(deadMinion) == 1)
            m_player1.season14.RecordReclaimedSoulsDeath(minion);
        else
            m_player2.season14.RecordReclaimedSoulsDeath(minion);

        // I'll Take That! records the first enemy minion killed by the
        // attacking player. Restrict capture to the attack-resolution pass;
        // deathrattle/simultaneous cleanup must not create a later copy.
        if (!beforeAttack) {
            if (std::get<0>(deadMinion) == 2 && m_turn == Turn::PLAYER1)
                m_player1.season14.RecordFirstKillCopy(minion);
            else if (std::get<0>(deadMinion) == 1 && m_turn == Turn::PLAYER2)
                m_player2.season14.RecordFirstKillCopy(minion);
        }

        if (std::get<0>(deadMinion) == 1)
        {
            if (!beforeAttack)
            {
                // If the zone position of minion that is destroyed is lower
                // than nextAttackerIdx and greater than 0, decrease by 1
                if (m_p1NextAttackerIdx < minion.GetZonePosition() &&
                    m_p1NextAttackerIdx > 0)
                {
                    --m_p1NextAttackerIdx;
                }
                // If the turn is player 1 and the zone position of minion that
                // is destroyed equals nextAttackerIdx, keep the value of it
                else if (m_turn == Turn::PLAYER1 &&
                         m_p1NextAttackerIdx == minion.GetZonePosition())
                {
                    isAttackerDestroyed = true;
                }
            }

            m_p1Field.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::DEATH, minion);
            });

            m_p2Field.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::DEATH, minion);
            });

            minion.SetLastFieldPos(minion.GetZonePosition());
            removedMinion = m_p1Field.Remove(minion);
        }
        else
        {
            if (!beforeAttack)
            {
                // If the zone position of minion that is destroyed is lower
                // than nextAttackerIdx and greater than 0, decrease by 1
                if (m_p2NextAttackerIdx < minion.GetZonePosition() &&
                    m_p2NextAttackerIdx > 0)
                {
                    --m_p2NextAttackerIdx;
                }
                // If the turn is player 2 and the zone position of minion that
                // is destroyed equals nextAttackerIdx, keep the value of it
                else if (m_turn == Turn::PLAYER2 &&
                         m_p2NextAttackerIdx == minion.GetZonePosition())
                {
                    isAttackerDestroyed = true;
                }
            }

            m_p1Field.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::DEATH, minion);
            });

            m_p2Field.ForEachAlive([&minion](MinionData& aliveMinion) {
                aliveMinion.value().ActivateTrigger(TriggerType::DEATH, minion);
            });

            minion.SetLastFieldPos(minion.GetZonePosition());
            removedMinion = m_p2Field.Remove(minion);
        }

        // Process deathrattle tasks
        if (removedMinion.HasDeathrattle())
        {
            removedMinion.ActivateTask(
                PowerType::DEATHRATTLE,
                std::get<0>(deadMinion) == 1 ? m_player1 : m_player2);
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            ++owner.season14.deathrattlesTriggered;
            owner.UpdateSkyGolemsForDeathrattle();
            owner.AdvanceDarkGiftCounters(2);
        }

        if (removedMinion.DeathrattleAttackTransfer() != 0 ||
            removedMinion.DeathrattleHealthTransfer() != 0)
        {
            FieldZone& ownerField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            bool transferred = false;
            ownerField.ForEachAlive([&](MinionData& data) {
                if (transferred) return;
                auto& target = data.value();
                target.SetAttack(target.GetAttack() + removedMinion.DeathrattleAttackTransfer());
                target.SetHealth(target.GetHealth() + removedMinion.DeathrattleHealthTransfer());
                // The gift specifies another friendly minion; the first live
                // recipient is the deterministic simulator selection.
                transferred = true;
            });
            removedMinion.SetDeathrattleStatTransfer(0, 0);
        }

        if (removedMinion.HasReborn())
        {
            FieldZone& ownerField =
                std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            Player& owner =
                std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;

            // Reborn is a one-shot property of the recruit-phase entity, not
            // just of this combat copy.  Consume it even when the board is
            // full and no revived copy can be summoned.
            ConsumeRebornInRecruitField(owner, removedMinion);
            if (!ownerField.IsFull())
            {
                removedMinion.ReviveWithReborn();
                int summonPosition = removedMinion.GetLastFieldPos();
                if (summonPosition > ownerField.GetCount())
                {
                    summonPosition = ownerField.GetCount();
                }
                ownerField.Add(removedMinion, summonPosition);

                // Reborn is a summon and therefore participates in existing
                // summon-trigger chains, while retaining the normal
                // deathrattle-before-Reborn ordering above.
                ownerField.ForEachAlive([&removedMinion](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::SUMMON,
                                                  removedMinion);
                });
                // Dispatch only after the revived entity is inserted and
                // alive. This preserves deathrattle -> Reborn -> summon
                // ordering and gives post-Reborn effects the real instance.
                ownerField.ForEachAlive([&removedMinion](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::REBORN,
                                                  removedMinion);
                });
                owner.ApplyAfterRebornTrinkets();
            }
        }

        // Lead the Frostwolves/Stormpikes use the same deterministic Avenge
        // lifecycle.  Resolve it only after deathrattle and Reborn handling so
        // a newly reborn friendly minion also receives the permanent bonus.
        Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
        auto& combatField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
        combatField.ForEachAlive([&owner](MinionData& data) {
            data.value().TriggerAvenge(owner);
        });
        const auto avenger = owner.season14.OnFriendlyMinionDiedBatch4();
        if (avenger.avengeTriggered)
        {
            ApplyPermanentAvengeBonus(
                owner,
                std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field,
                avenger.attack, avenger.health);
        }
    }

    if (!beforeAttack)
    {
        // If the zone position of minion that is destroyed not equals
        // nextAttackerIdx, increase by 1
        if (!isAttackerDestroyed)
        {
            if (m_turn == Turn::PLAYER1)
            {
                ++m_p1NextAttackerIdx;
            }
            else
            {
                ++m_p2NextAttackerIdx;
            }
        }

        // Check the boundaries of field zone
        if (m_p1NextAttackerIdx >= m_p1Field.GetCount())
        {
            m_p1NextAttackerIdx %= std::max(1, m_p1Field.GetCount());
        }
        if (m_p2NextAttackerIdx >= m_p2Field.GetCount())
        {
            m_p2NextAttackerIdx %= std::max(1, m_p2Field.GetCount());
        }
    }
}

bool Battle::IsDone() const
{
    return m_p1Field.IsEmpty() || m_p2Field.IsEmpty() || m_turn == Turn::DONE;
}

void Battle::ProcessResult()
{
    if (m_p1Field.IsEmpty() && !m_p2Field.IsEmpty())
    {
        m_result = BattleResult::PLAYER2_WIN;
    }
    else if (!m_p1Field.IsEmpty() && m_p2Field.IsEmpty())
    {
        m_result = BattleResult::PLAYER1_WIN;
    }
    else
    {
        m_result = BattleResult::DRAW;
    }
}

int Battle::CalculateDamage()
{
    int totalDamage = 0;

    if (m_result == BattleResult::PLAYER1_WIN)
    {
        m_p1Field.ForEach([&totalDamage](const MinionData& minion) {
            totalDamage += minion.value().GetTier();
        });

        totalDamage += m_player1.currentTier;
    }
    else
    {
        m_p2Field.ForEach([&totalDamage](const MinionData& minion) {
            totalDamage += minion.value().GetTier();
        });

        totalDamage += m_player2.currentTier;
    }

    return totalDamage;
}

FieldZone& Battle::GetPlayer1Field()
{
    return m_p1Field;
}

const FieldZone& Battle::GetPlayer1Field() const
{
    return m_p1Field;
}

FieldZone& Battle::GetPlayer2Field()
{
    return m_p2Field;
}

const FieldZone& Battle::GetPlayer2Field() const
{
    return m_p2Field;
}

int Battle::GetPlayer1NextAttacker() const
{
    return m_p1NextAttackerIdx;
}

int Battle::GetPlayer2NextAttacker() const
{
    return m_p2NextAttackerIdx;
}

BattleResult Battle::GetResult() const
{
    return m_result;
}
}  // namespace RosettaStone::Battlegrounds
