// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Models/Battle.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
Battle::Battle(Player& player1, Player& player2)
    : m_player1(player1),
      m_player2(player2),
      m_p1Field(m_player1.battleField),
      m_p2Field(m_player2.battleField)
{
    m_player1.battleField = m_player1.recruitField;
    m_player2.battleField = m_player2.recruitField;
}

void Battle::Initialize()
{
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
        minion.value().ApplyStartCombatStatMultipliers();
    });
    m_p2Field.ForEach([](MinionData& minion) {
        minion.value().ApplyStartCombatStatMultipliers();
    });

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

void Battle::Run()
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
        m_player2.hero.TakeDamage(m_player2, damage);
    }
    else if (m_result == BattleResult::PLAYER2_WIN)
    {
        m_player1.hero.TakeDamage(m_player1, damage);
    }
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

    int& pendingAttacks = (m_turn == Turn::PLAYER1)
                              ? m_p1PendingAttacks
                              : m_p2PendingAttacks;
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
    target.TakeDamage(attacker);
    attacker.TakeDamage(target);
    const bool targetWasDestroyed = target.IsDestroyed();

    ProcessDestroy(false);

    // A Reborn copy has already spent its attack.  Do not mistake it for the
    // original Windfury attacker when a minion dies in combat or a death
    // trigger removes it before ProcessDestroy completes.
    bool attackerSurvived = false;
    int attackerPositionAfterCleanup = -1;
    FieldZone& attackerField = (m_turn == Turn::PLAYER1) ? m_p1Field
                                                         : m_p2Field;
    attackerField.ForEachAlive([&](MinionData& minionData) {
        const Minion& candidate = minionData.value();
        const bool sameEntity = attackerEntityIndex >= 0
                                    ? candidate.GetIndex() == attackerEntityIndex
                                    : (candidate.GetZonePosition() ==
                                           attackerZonePosition &&
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
        const auto bonus = (m_turn == Turn::PLAYER1)
                               ? m_player1.season14
                                     .HeroPowerBatch3CombatKillAttackBonus()
                               : m_player2.season14
                                     .HeroPowerBatch3CombatKillAttackBonus();
        if (bonus > 0)
        {
            attackerField.ForEachAlive([&](MinionData& minionData) {
                Minion& candidate = minionData.value();
                const bool sameEntity = attackerEntityIndex >= 0
                                            ? candidate.GetIndex() ==
                                                  attackerEntityIndex
                                            : (candidate.GetZonePosition() ==
                                                   attackerZonePosition &&
                                               candidate.GetCardID() ==
                                                   attackerCardID);
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

    std::vector<std::size_t> tauntMinions;
    tauntMinions.reserve(MAX_FIELD_SIZE);

    std::size_t minionIdx = 0;
    minions.ForEach([&tauntMinions, &minionIdx](const MinionData& minion) {
        if (minion.value().HasTaunt())
        {
            tauntMinions.emplace_back(minionIdx);
        }

        ++minionIdx;
    });

    if (!tauntMinions.empty())
    {
        const auto idx = Random::get<std::size_t>(0, tauntMinions.size() - 1);
        return minions[tauntMinions[idx]];
    }

    const auto idx = Random::get<int>(0, minions.GetCount() - 1);
    return minions[idx];
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
        }

        if (removedMinion.HasReborn())
        {
            FieldZone& ownerField = std::get<0>(deadMinion) == 1
                                        ? m_p1Field
                                        : m_p2Field;
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
            }
        }

        // Lead the Frostwolves/Stormpikes use the same deterministic Avenge
        // lifecycle.  Resolve it only after deathrattle and Reborn handling so
        // a newly reborn friendly minion also receives the permanent bonus.
        Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
        const auto avenger = owner.season14.OnFriendlyMinionDiedBatch4();
        if (avenger.avengeTriggered)
        {
            owner.recruitField.ForEachAlive(
                [&avenger](MinionData& aliveMinion) {
                    aliveMinion.value().SetAttack(
                        aliveMinion.value().GetAttack() + avenger.attack);
                    aliveMinion.value().SetHealth(
                        aliveMinion.value().GetHealth() + avenger.health);
                });
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
