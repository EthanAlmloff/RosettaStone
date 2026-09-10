// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/CardSets/BuddyBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/QuilboarBloodGolemDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTavernSpellToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
namespace
{
void ApplyEmbraceElement(Player& owner, FieldZone& friendly, FieldZone& enemy,
                         std::int32_t element)
{
    if (element == 79721) {
        std::vector<Minion*> candidates;
        friendly.ForEachAlive([&](MinionData& data) { candidates.push_back(&data.value()); });
        Random::shuffle(candidates.begin(), candidates.end());
        for (std::size_t i = 0; i < std::min<std::size_t>(4, candidates.size()); ++i)
            candidates[i]->SetEarthElementalDeathrattle(true);
    } else if (element == 79722) {
        Minion* left = nullptr;
        friendly.ForEachAlive([&](MinionData& data) { if (left == nullptr) left = &data.value(); });
        if (left != nullptr) left->SetAttack(left->GetAttack() * 2);
    } else if (element == 79723) {
        Minion* right = nullptr;
        friendly.ForEachAlive([&](MinionData& data) { right = &data.value(); });
        if (right != nullptr) { right->SetHealth(right->GetHealth() + 3); right->SetTaunt(true); }
    } else if (element == 79724) {
        std::vector<Minion*> candidates;
        enemy.ForEachAlive([&](MinionData& data) { candidates.push_back(&data.value()); });
        Random::shuffle(candidates.begin(), candidates.end());
        for (std::size_t i = 0; i < std::min<std::size_t>(5, candidates.size()); ++i)
            candidates[i]->SetHealth(candidates[i]->GetHealth() - 1);
    }
}

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
    m_player1.season14.ClearCombatDeadMinions();
    m_player2.season14.ClearCombatDeadMinions();
    m_player1.season14.ClearBoomControllerMech();
    m_player2.season14.ClearBoomControllerMech();
    m_player1.recruitField.ForEachAlive([](MinionData& data) { data.value().BeginPoetCombatSnapshot(false); });
    m_player2.recruitField.ForEachAlive([](MinionData& data) { data.value().BeginPoetCombatSnapshot(false); });
    // Tarecgosa Sticker arms the current left/right-most friendly Dragons at
    // the combat boundary.  The marker lives on the entity, so the normal
    // combat-copy/reconciliation path permanently retains later combat stats
    // and bonus keywords.  Existing Tarecgosa Blessing markers (for example
    // from a Dark Gift) are intentionally left untouched.
    const auto armTarecgosaSticker = [](Player& owner) {
        bool active = false;
        for (const auto& trinket : owner.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::TARECGOSA_STICKER) {
                active = true;
                break;
            }
        }
        if (!active) return;
        std::vector<Minion*> dragons;
        owner.recruitField.ForEachAlive([&dragons](MinionData& data) {
            if (data.value().HasRace(Race::DRAGON))
                dragons.push_back(&data.value());
        });
        std::sort(dragons.begin(), dragons.end(),
                  [](const Minion* lhs, const Minion* rhs) {
                      return lhs->GetZonePosition() < rhs->GetZonePosition();
                  });
        if (!dragons.empty()) dragons.front()->SetTarecgosaBlessing();
        if (dragons.size() > 1) dragons.back()->SetTarecgosaBlessing();
    };
    armTarecgosaSticker(m_player1);
    armTarecgosaSticker(m_player2);
    m_player1.battleField = m_player1.recruitField;
    m_player2.battleField = m_player2.recruitField;
    // Powder Keg is a combat-copy aura.  Arm only the first three friendly
    // Pirates at the combat boundary; the recruit entities remain untouched
    // until a real persistent effect is committed by the normal battle path.
    const auto armPowderKeg = [](Player& owner) {
        int remaining = 0;
        for (const auto& trinket : owner.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::START_COMBAT_POWDER_KEG)
                remaining += behavior.value;
        }
        if (remaining == 0) return;
        std::vector<Minion*> pirates;
        owner.battleField.ForEachAlive([&pirates](MinionData& data) {
            if (data.value().HasRace(Race::PIRATE))
                pirates.push_back(&data.value());
        });
        // "Give 3 friendly Pirates" selects distinct eligible entities; the
        // board order must not turn the effect into a leftmost-only buff.
        Random::shuffle(pirates.begin(), pirates.end());
        for (auto* pirate : pirates) {
            if (remaining-- == 0) break;
            pirate->SetPowderKegDeathrattleAttack(pirate->GetAttack());
        }
    };
    armPowderKeg(m_player1);
    armPowderKeg(m_player2);
    // Caduceus Reactor is a dynamic Deathrattle payload on the copied
    // Battlecruiser.  Arm the combat entity only; Battle's normal destroy
    // pipeline then applies the transfer to the left-most surviving minion.
    const auto armLiftOffReactor = [](Player& owner) {
        if (owner.season14.liftOffDeathrattleAttack == 0 &&
            owner.season14.liftOffDeathrattleHealth == 0) return;
        owner.battleField.ForEachAlive([&owner](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG31_HERO_801pt" ||
                minion.GetCardID() == "BG31_HERO_801pt_G")
                minion.SetDeathrattleStatTransfer(
                    owner.season14.liftOffDeathrattleAttack,
                    owner.season14.liftOffDeathrattleHealth);
        });
    };
    armLiftOffReactor(m_player1);
    armLiftOffReactor(m_player2);
    const auto armLiftOffUltraCapacitor = [](Player& owner) {
        if (!owner.season14.liftOffUltraCapacitor) return;
        owner.battleField.ForEachAlive([](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "BG31_HERO_801pt" ||
                minion.GetCardID() == "BG31_HERO_801pt_G") {
                minion.SetReborn(true);
                minion.SetRebornFullHealth(true);
            }
        });
    };
    armLiftOffUltraCapacitor(m_player1);
    armLiftOffUltraCapacitor(m_player2);
    // Elder Taggawag snapshots the warband's distinct races at the combat
    // boundary.  The gain is combat-only, so apply it to the copied Buddy and
    // never commit it back to recruitField.  Exclude the source Buddy from
    // the maxima to avoid self-referential stat growth.
    const auto applyTaggawag = [](FieldZone& field) {
        std::array<Race, 8> seen{};
        std::size_t seenCount = 0;
        int highestAttack = 0;
        int highestHealth = 0;
        Minion* taggawag = nullptr;
        field.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            if (minion.GetCardID() == "TB_BaconShop_HERO_14_Buddy" ||
                minion.GetCardID() == "TB_BaconShop_HERO_14_Buddy_G") {
                taggawag = &minion;
                return;
            }
            const auto race = minion.GetRace();
            if (race != Race::INVALID &&
                std::find(seen.begin(), seen.begin() + seenCount, race) ==
                    seen.begin() + seenCount) {
                if (seenCount < seen.size()) seen[seenCount++] = race;
            }
            highestAttack = std::max(highestAttack, minion.GetAttack());
            highestHealth = std::max(highestHealth, minion.GetHealth());
        });
        if (taggawag == nullptr || seenCount < 4) return;
        const int multiplier = taggawag->GetCardID().ends_with("_G") ? 2 : 1;
        taggawag->SetAttack(taggawag->GetAttack() + highestAttack * multiplier);
        taggawag->SetHealth(taggawag->GetHealth() + highestHealth * multiplier);
    };
    applyTaggawag(m_player1.battleField);
    applyTaggawag(m_player2.battleField);
    const auto snapshotPoets = [](FieldZone& field) {
        std::vector<Minion*> minions;
        field.ForEachAlive([&](MinionData& data) { minions.push_back(&data.value()); });
        // Timewarped Poet (the Poet Portrait reward) extends the existing
        // combat-persistence snapshot to every friendly Dragon.  Its golden
        // copy doubles the combat gains, just as golden Persistent Poet does
        // for adjacent Dragons.
        bool hasTimewarpedPoet = false;
        int timewarpedMultiplier = 1;
        for (auto* candidate : minions) {
            if (candidate->GetCardID() == "BG34_Giant_314")
                hasTimewarpedPoet = true;
            else if (candidate->GetCardID() == "BG34_Giant_314_G") {
                hasTimewarpedPoet = true;
                timewarpedMultiplier = 2;
            }
        }
        for (auto* minion : minions) {
            bool eligible = hasTimewarpedPoet && minion->HasRace(Race::DRAGON);
            int multiplier = hasTimewarpedPoet ? timewarpedMultiplier : 1;
            for (auto* candidate : minions) {
                if (candidate->GetCardID() != "BG29_813" && candidate->GetCardID() != "BG29_813_G") continue;
                if (std::abs(candidate->GetZonePosition() - minion->GetZonePosition()) != 1) continue;
                if (minion->HasRace(Race::DRAGON)) {
                    eligible = true;
                    if (candidate->GetCardID() == "BG29_813_G") multiplier = 2;
                }
            }
            minion->BeginPoetCombatSnapshot(eligible, multiplier);
        }
    };
    snapshotPoets(m_p1Field);
    snapshotPoets(m_p2Field);
    const auto summonHandSnapshot = [](Player& owner, FieldZone& field) {
        auto snapshots = owner.season14.TakeCombatHandSummons();
        for (auto& [snapshot, count] : snapshots) {
            for (int i = 0; i < count; ++i) {
                if (field.IsFull()) {
                    owner.ApplySummonOverflowTrinkets();
                    break;
                }
                Minion copy{snapshot};
                // A combat-only copy is a fresh entity.  Preserve the full
                // card instance (stats, keywords, enchantments and tasks),
                // but give it the owning player's callback and a new entity
                // index so trigger/source identity cannot alias the hand
                // instance or another summoned copy.
                copy.getPlayerCallback = [&owner]() -> Player& { return owner; };
                if (owner.getNextCardIndexCallback)
                    copy.SetIndex(owner.getNextCardIndexCallback());
                field.Add(copy);
                Minion& summoned = field[field.GetCount() - 1];
                field.ForEachAlive([&summoned](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::SUMMON,
                                                   summoned);
                });
                owner.ApplySummonTrinkets(summoned);
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
        for (std::size_t cast = 0; cast < casts; ++cast)
            for (int i = 0; i < 2; ++i)
            {
                if (field.IsFull()) {
                    owner.ApplySummonOverflowTrinkets();
                    break;
                }
                Minion beetle{ beetleCard };
                owner.ApplyFreshMinionModifiers(beetle);
                field.Add(beetle);
                Minion& summoned = field[field.GetCount() - 1];
                field.ForEachAlive([&summoned](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::SUMMON,
                                                   summoned);
                });
                owner.ApplySummonTrinkets(summoned);
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
            {
                // Sr. Tomb Diver (and other permanent goldenization effects)
                // changes the combat copy's card identity. Reconcile that
                // identity back to the recruit entity; otherwise the effect
                // appears to work during combat but silently reverts at the
                // next recruit phase. Temporary golden conversions are
                // deliberately excluded and expire through their normal
                // lifecycle.
                if (combat->IsGolden() && !combat->IsTemporarilyGolden() &&
                    !recruit.IsGolden() && recruit.CanMakeGolden())
                    (void)recruit.MakeGolden();
                recruit.ReconcileCombatPersistentState(*combat);
                // Goldrinn's Soul of the Beast lasts through combat and
                // expires at the next recruit start. Temporary combat deltas
                // normally do not reconcile; transfer only marked Goldrinn
                // occurrences to the recruit copy.
                const auto& temporary = combat->GetTemporaryEnchantments();
                const auto goldrinn = std::count_if(
                    temporary.begin(), temporary.end(),
                    [](const std::string& id) { return id == "BGS_018e"; });
                if (goldrinn > 0)
                {
                    recruit.ApplyTemporaryEnchantment(
                        Minion::TemporaryEnchantment::Stats,
                        static_cast<int>(goldrinn) * 8,
                        static_cast<int>(goldrinn) * 8);
                    for (int i = 0; i < goldrinn; ++i)
                        recruit.RecordTemporaryEnchantmentOccurrence("BGS_018e");
                }
                if (combat->IsPoetCombatEligible()) {
                    const int attackGain = std::max(0, combat->GetAttack() - combat->PoetCombatAttack()) * combat->PoetCombatMultiplier();
                    const int healthGain = std::max(0, combat->GetHealth() - combat->PoetCombatHealth()) * combat->PoetCombatMultiplier();
                    if (attackGain || healthGain)
                        recruit.ApplyPersistentMinionStats(attackGain, healthGain);
                    const auto newlyGained = combat->PoetCombatKeywords();
                    const auto baseline = recruit.PoetCombatKeywords();
                    const auto keywords = newlyGained & ~baseline;
                    if (keywords & (1u << 0)) recruit.ApplyCombatPersistentKeyword(GameTag::TAUNT);
                    if (keywords & (1u << 1)) recruit.ApplyCombatPersistentKeyword(GameTag::DIVINE_SHIELD);
                    if (keywords & (1u << 2)) recruit.ApplyCombatPersistentKeyword(GameTag::REBORN);
                    if (keywords & (1u << 3)) recruit.ApplyCombatPersistentKeyword(GameTag::WINDFURY);
                    if (keywords & (1u << 4)) recruit.ApplyCombatPersistentKeyword(GameTag::MEGA_WINDFURY);
                    if (keywords & (1u << 5)) recruit.ApplyCombatPersistentKeyword(GameTag::VENOMOUS);
                }
            }
        });
    };
    commit(m_player1, m_p1Field);
    commit(m_player2, m_p2Field);
}

void Battle::Initialize()
{
    m_p1EclipsionAttacks = 0;
    m_p2EclipsionAttacks = 0;
    m_shadowyConstructTriggers.clear();
    const auto resolveCarrierInterceptors = [this](Player& owner,
                                                    FieldZone& own,
                                                    FieldZone& enemy) {
        const int count = owner.season14.carrierInterceptors;
        owner.season14.carrierInterceptors = 0;
        const auto card = Cards::FindCardByDbfID(113175);
        if (count <= 0 || card.dbfID == 0) return;
        for (int i = 0; i < count; ++i) {
            if (own.IsFull()) {
                owner.ApplySummonOverflowTrinkets();
                break;
            }
            Minion interceptor(card);
            owner.ApplyFreshMinionModifiers(interceptor);
            interceptor.getPlayerCallback = [&owner]() -> Player& { return owner; };
            if (owner.getNextCardIndexCallback)
                interceptor.SetIndex(owner.getNextCardIndexCallback());
            own.Add(interceptor, own.GetCount());
            Minion& added = own[own.GetCount() - 1];
            own.ForEachAlive([&added](MinionData& data) {
                data.value().ActivateTrigger(TriggerType::SUMMON, added);
            });
            if (enemy.GetCount() == 0) continue;
            std::vector<Minion*> targets;
            enemy.ForEachAlive([&targets](MinionData& data) {
                if (!data.value().HasStealth()) targets.push_back(&data.value());
            });
            if (targets.empty()) continue;
            auto* target = targets[Random::get<std::size_t>(0, targets.size() - 1)];
            target->TakeDamage(added);
            ProcessDestroy(true);
        }
    };
    resolveCarrierInterceptors(m_player1, m_p1Field, m_p2Field);
    resolveCarrierInterceptors(m_player2, m_p2Field, m_p1Field);
    const auto fireLockAndLoad = [this](Player& owner, FieldZone& own,
                                        FieldZone& enemy) {
        const auto before = own.GetCount();
        owner.ResolveLockAndLoad();
        if (own.GetCount() <= before || !HasAttackableTarget(enemy)) return;
        Minion& projectile = own[own.GetCount() - 1];
        std::vector<Minion*> targets;
        enemy.ForEachAlive([&targets](MinionData& data) {
            if (!data.value().HasStealth()) targets.push_back(&data.value());
        });
        if (targets.empty()) return;
        auto& target = *targets[Random::get<std::size_t>(0, targets.size() - 1)];
        {
            AttackingStateGuard attacking(projectile);
            target.TakeDamage(projectile);
            projectile.TakeDamage(target);
        }
        m_killContext = KillContext{projectile.GetIndex(),
                                    &owner == &m_player1 ? 1 : 2,
                                    true};
        ProcessDestroy(false);
        m_killContext = {};
    };
    fireLockAndLoad(m_player1, m_p1Field, m_p2Field);
    fireLockAndLoad(m_player2, m_p2Field, m_p1Field);
    // Stolen Gold is a player-owned quest reward.  Resolve it on the combat
    // copies so the temporary golden conversion cannot mutate recruit state.
    m_player1.ResolveGeneratedQuestRewardStartCombat(m_p1Field);
    m_player2.ResolveGeneratedQuestRewardStartCombat(m_p2Field);
    m_p1Field.ForEachAlive([](MinionData& data) { data.value().ResetAvengeProgress(); });
    m_p2Field.ForEachAlive([](MinionData& data) { data.value().ResetAvengeProgress(); });
    m_player1.season14.TakeCombatAvengeCards();
    m_player2.season14.TakeCombatAvengeCards();
    m_player1.season14.BeginCombatBatch4();
    m_player2.season14.BeginCombatBatch4();
    m_player1.season14.ResetGeneratedRewardAvenge();
    m_player2.season14.ResetGeneratedRewardAvenge();
    m_player1.season14.ResetGeneratedRewardTumblingAvenge();
    m_player2.season14.ResetGeneratedRewardTumblingAvenge();
    m_player1.season14.ResetTrinketAvengeProgress();
    m_player2.season14.ResetTrinketAvengeProgress();
    for (auto& trinket : m_player1.season14.trinkets)
        if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
            TrinketEffect::START_COMBAT_FIRST_SUMMON_COPY) {
            trinket.triggerProgress = 0;
            trinket.pendingFirstSummon.reset();
        }
    for (auto& trinket : m_player2.season14.trinkets)
        if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
            TrinketEffect::START_COMBAT_FIRST_SUMMON_COPY) {
            trinket.triggerProgress = 0;
            trinket.pendingFirstSummon.reset();
        }
    for (auto* owner : {&m_player1, &m_player2})
        for (auto& trinket : owner->season14.trinkets) {
            const auto effect = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id).effect;
            if (effect == TrinketEffect::AFTER_LAST_FRIENDLY_DEATH_DEMON ||
                FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id)
                        .portraitEffect == PortraitEffect::TIDE_RAISER_COMBAT_SPELL_COPY)
                trinket.triggerProgress = 0;
        }
    m_player1.season14.ResetBroodmotherAvenge();
    m_player2.season14.ResetBroodmotherAvenge();
    m_player1.ApplyStartCombatTrinkets();
    m_player2.ApplyStartCombatTrinkets();

    // Tentacular summons a combat-only 2/2 Taunt Tentacle at Start of
    // Combat.  The token is omitted when the copied board is full; otherwise
    // it enters at the right edge and SUMMON observers see the fresh entity.
    const auto summonTentacle = [](Player& owner, FieldZone& field) {
        if (owner.season14.heroPowerDbfID != 86014) return;
        if (field.IsFull()) {
            owner.ApplySummonOverflowTrinkets();
            return;
        }
        const Card token = Cards::FindCardByDbfID(86227);
        if (token.id.empty()) return;
        Minion summoned(token);
        summoned.SetAttack(summoned.GetAttack() + owner.season14.tentacularBonus);
        summoned.SetHealth(summoned.GetHealth() + owner.season14.tentacularBonus);
        summoned.getPlayerCallback = [&owner]() -> Player& { return owner; };
        if (owner.getNextCardIndexCallback)
            summoned.SetIndex(owner.getNextCardIndexCallback());
        field.Add(summoned, field.GetCount());
        Minion& added = field[field.GetCount() - 1];
        field.ForEachAlive([&added](MinionData& data) {
            data.value().ActivateTrigger(TriggerType::SUMMON, added);
        });
        owner.ApplySummonTrinkets(added);
    };
    summonTentacle(m_player1, m_p1Field);
    summonTentacle(m_player2, m_p2Field);

    // Murloc King arms every friendly combat copy with the exact generated
    // Murloc Scout deathrattle.  The arm is consumed after copying so the
    // effect applies only to the next combat.
    const auto armMurlocKing = [](Player& owner, FieldZone& field) {
        const auto copies = owner.season14.heroPowerBatch8.murlocKingActivations > 0
                                ? owner.season14.heroPowerBatch8.murlocKingActivations
                                : (owner.season14.heroPowerBatch8.murlocKingPending ? 1 : 0);
        if (copies == 0) return;
        field.ForEachAlive([copies](MinionData& data) {
            for (int i = 0; i < copies; ++i)
                data.value().AddDarkGiftDeathrattleTask(
                    TaskType{SimpleTasks::SummonTask{"EX1_506a", 1}});
        });
        owner.season14.heroPowerBatch8.murlocKingPending = false;
        owner.season14.heroPowerBatch8.murlocKingActivations = 0;
    };
    armMurlocKing(m_player1, m_p1Field);
    armMurlocKing(m_player2, m_p2Field);

    // These hero powers use the simulator's seeded random-target task, so
    // target choice and damage/death ordering remain identical to card tasks.
    const auto resolveBatch8CombatPower = [](Player& owner) {
        const auto dbfID = owner.season14.heroPowerBatch8.pendingStartCombatPower;
        if (dbfID == 0) return;
        const auto repeats = std::max(1, owner.season14.heroPowerBatch8.pendingStartCombatPowerRepeats);
        owner.season14.heroPowerBatch8.pendingStartCombatPower = 0;
        owner.season14.heroPowerBatch8.pendingStartCombatPowerRepeats = 0;
        Season14HeroPowerBatch8Result effect{};
        if (!ResolveSeason14HeroPowerBatch8CombatStart(dbfID, effect)) return;
        Minion source;
        for (int repeat = 0; repeat < repeats; ++repeat) {
            if (effect.allEnemyMinions) {
                Player& opponent = owner.getOpponentPlayerCallback(owner);
                opponent.GetField().ForEachAlive([&](MinionData& data) {
                    data.value().TakeDamage(effect.damage);
                });
            } else {
                SimpleTasks::RandomEnemyDamageTask task{effect.damage,
                                                        effect.damageTargets};
                task.Run(owner, source);
            }
        }
    };
    resolveBatch8CombatPower(m_player1);
    resolveBatch8CombatPower(m_player2);

    const auto applyLeftKeywords = [](Player& owner, FieldZone& field) {
        if (owner.season14.heroPowerDbfID != 64402) return;
        Minion* left = nullptr;
        field.ForEachAlive([&](MinionData& data) { if (left == nullptr) left = &data.value(); });
        if (left != nullptr) {
            left->ApplyTemporaryKeyword(GameTag::WINDFURY);
            left->ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
            left->SetTaunt(true);
        }
    };
    applyLeftKeywords(m_player1, m_p1Field);
    applyLeftKeywords(m_player2, m_p2Field);

    const auto resolveEmbrace = [](Player& owner, FieldZone& friendly,
                                   FieldZone& enemy) {
        ApplyEmbraceElement(owner, friendly, enemy,
                            owner.season14.embraceElementDbfID);
    };
    resolveEmbrace(m_player1, m_p1Field, m_p2Field);
    resolveEmbrace(m_player2, m_p2Field, m_p1Field);

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

    // Fragrant Phylactery is a passive start-of-combat effect, not an active
    // Hero Power activation: Ancient Wishbone must not duplicate it.  It
    // destroys the lowest-Health friendly combat copy,
    // then gives its stats to up to five surviving friendly minions.  Select
    // after start-of-combat stat modifiers; ties are random.  Resolve the
    // destruction through the normal death pipeline before choosing targets,
    // so deathrattles and board-space semantics remain authoritative.
    const auto resolveFragrantPhylactery = [this](Player& owner,
                                                   FieldZone& field) {
        if (owner.season14.heroPowerDbfID != 77911) return;
        std::vector<Minion*> lowest;
        int lowestHealth = 0;
        field.ForEachAlive([&](MinionData& data) {
            auto& minion = data.value();
            if (lowest.empty() || minion.GetHealth() < lowestHealth) {
                lowest.clear();
                lowestHealth = minion.GetHealth();
            }
            if (minion.GetHealth() == lowestHealth)
                lowest.push_back(&minion);
        });
        if (lowest.empty()) return;
        Minion& selected = *lowest[Random::get<std::size_t>(0, lowest.size() - 1)];
        const int attack = selected.GetAttack();
        const int health = selected.GetHealth();
        selected.SetHealth(0);
        ProcessDestroy(true);

        std::vector<Minion*> survivors;
        field.ForEachAlive([&survivors](MinionData& data) {
            survivors.push_back(&data.value());
        });
        Random::shuffle(survivors.begin(), survivors.end());
        const auto count = std::min<std::size_t>(5, survivors.size());
        for (std::size_t i = 0; i < count; ++i) {
            survivors[i]->SetAttack(survivors[i]->GetAttack() + attack);
            survivors[i]->SetHealth(survivors[i]->GetHealth() + health);
        }
    };
    resolveFragrantPhylactery(m_player1, m_p1Field);
    resolveFragrantPhylactery(m_player2, m_p2Field);

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

    // Yamato Cannon is attached to the recruit-side Battlecruiser but fires
    // from the combat copy.  Select the highest-health enemy independently
    // for each trigger (Missile Pod supplies the second trigger), preserving
    // the simulator's normal damage/death pipeline.
    const auto resolveLiftOffYamato = [this](Player& owner, FieldZone& enemy) {
        if (owner.season14.liftOffYamatoDamage <= 0) return;
        const int repeats = owner.season14.liftOffMissilePod ? 2 : 1;
        for (int repeat = 0; repeat < repeats; ++repeat) {
            Minion* highest = nullptr;
            enemy.ForEachAlive([&highest](MinionData& data) {
                auto& candidate = data.value();
                if (highest == nullptr || candidate.GetHealth() > highest->GetHealth())
                    highest = &candidate;
            });
            if (highest == nullptr) break;
            highest->TakeDamage(owner.season14.liftOffYamatoDamage);
            ProcessDestroy(true);
        }
    };
    resolveLiftOffYamato(m_player1, m_p2Field);
    resolveLiftOffYamato(m_player2, m_p1Field);

    // Start-of-Combat is a pass over the combat board as it existed when the
    // pass began.  Snapshot entity IDs before dispatch: a start effect may
    // summon a minion, and visiting that new minion in the same pass would
    // make its effect timing depend on array capacity/iteration details.
    // The snapshot is also required by Wind Chimes below: its extra trigger
    // must repeat the original effects once, rather than recursively
    // re-visiting minions created by the first pass.
    const auto snapshotStartCombat = [](FieldZone& field) {
        std::vector<std::uint64_t> entities;
        field.ForEach([&entities](MinionData& data) {
            entities.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
        });
        return entities;
    };
    const auto p1StartCombatEntities = snapshotStartCombat(m_p1Field);
    const auto p2StartCombatEntities = snapshotStartCombat(m_p2Field);
    const auto runStartCombat = [](Player& owner, FieldZone& field,
                                   const std::vector<std::uint64_t>& entities) {
        for (const auto entityID : entities) {
            for (int i = 0; i < field.GetCount(); ++i) {
                auto& minion = field[static_cast<std::size_t>(i)];
                if (static_cast<std::uint64_t>(minion.GetIndex()) != entityID ||
                    minion.IsDestroyed())
                    continue;
                minion.ActivateTask(PowerType::START_OF_COMBAT, owner);
                break;
            }
        }
    };
    if (m_turn == Turn::PLAYER1)
    {
        runStartCombat(m_player1, m_p1Field, p1StartCombatEntities);
        runStartCombat(m_player2, m_p2Field, p2StartCombatEntities);
    }
    else
    {
        runStartCombat(m_player2, m_p2Field, p2StartCombatEntities);
        runStartCombat(m_player1, m_p1Field, p1StartCombatEntities);
    }

    m_player1.ResolveTierMinionStartCombat();
    m_player2.ResolveTierMinionStartCombat();

    // Valdrakken Wind Chimes repeats every friendly Start-of-Combat effect
    // once after the ordinary pass. Reuse the authoritative task dispatch
    // so card-specific ordering and generated effects remain intact.
    const auto repeatStartCombat = [](Player& owner, FieldZone& field,
                                      const std::vector<std::uint64_t>& entities) {
        bool repeatAllEffects = false;
        int repeatFirstEffectCount = 0;
        for (const auto& trinket : owner.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::START_COMBAT_EXTRA_TRIGGER)
                repeatAllEffects = true;
            else if (behavior.portraitEffect ==
                     PortraitEffect::PROMO_START_COMBAT_EXTRA_TRIGGER)
                ++repeatFirstEffectCount;
        }
        std::optional<std::uint64_t> firstStartCombatEntity;
        for (const auto entityID : entities) {
            for (int i = 0; i < field.GetCount(); ++i) {
                auto& minion = field[static_cast<std::size_t>(i)];
                if (static_cast<std::uint64_t>(minion.GetIndex()) != entityID ||
                    minion.IsDestroyed() ||
                    minion.GetTasks(PowerType::START_OF_COMBAT).empty())
                    continue;
                firstStartCombatEntity = entityID;
                break;
            }
            if (firstStartCombatEntity.has_value()) break;
        }
        if (repeatAllEffects) {
            for (const auto entityID : entities) {
                for (int i = 0; i < field.GetCount(); ++i) {
                    auto& minion = field[static_cast<std::size_t>(i)];
                    if (static_cast<std::uint64_t>(minion.GetIndex()) != entityID ||
                        minion.IsDestroyed())
                        continue;
                    minion.ActivateTask(PowerType::START_OF_COMBAT, owner);
                    break;
                }
            }
        }
        // Promo Portrait is intentionally narrower than Wind Chimes: each
        // active portrait repeats only the first actual Start-of-Combat
        // effect in the original board snapshot.  Checking the task list
        // avoids treating a preceding vanilla minion with no effect as the
        // printed "first" effect, and the snapshot prevents newly summoned
        // minions from becoming eligible in this same extra pass.
        for (int repeat = 0; repeat < repeatFirstEffectCount; ++repeat) {
            if (!firstStartCombatEntity.has_value()) break;
            for (int i = 0; i < field.GetCount(); ++i) {
                auto& minion = field[static_cast<std::size_t>(i)];
                if (static_cast<std::uint64_t>(minion.GetIndex()) !=
                        *firstStartCombatEntity ||
                    minion.IsDestroyed())
                    continue;
                minion.ActivateTask(PowerType::START_OF_COMBAT, owner);
                break;
            }
        }
    };
    repeatStartCombat(m_player1, m_p1Field, p1StartCombatEntities);
    repeatStartCombat(m_player2, m_p2Field, p2StartCombatEntities);

    // Illidan's start-of-combat power buffs the current edge minions and
    // queues each edge for one immediate attack.  Resolve these attacks after
    // all start-of-combat tasks, using stable entity IDs so death cleanup can
    // safely remove either edge before the next one is reacquired.
    const auto resolveYoHoOgre = [this](Player& owner, FieldZone& friendly,
                                        FieldZone& enemy, Turn turn) {
        if (owner.season14.heroPowerDbfID != 61851) return;
        std::vector<std::uint64_t> edges;
        friendly.ForEachAlive([&](MinionData& data) {
            edges.push_back(static_cast<std::uint64_t>(data.value().GetIndex()));
        });
        if (edges.empty()) return;
        if (edges.size() > 1) {
            const auto right = edges.back();
            edges.resize(2);
            edges[1] = right;
        }
        m_turn = turn;
        for (const auto entityID : edges) {
            Minion* attacker = nullptr;
            friendly.ForEachAlive([&](MinionData& data) {
                if (static_cast<std::uint64_t>(data.value().GetIndex()) == entityID)
                    attacker = &data.value();
            });
            if (!attacker) continue;
            attacker->SetAttack(attacker->GetAttack() + 2);
            attacker->SetHealth(attacker->GetHealth() + 1);
            if (!HasAttackableTarget(enemy)) continue;
            auto& target = GetProperTarget(*attacker);
            {
                AttackingStateGuard attacking(*attacker);
                owner.season14.OnFriendlyMinionAttack();
                owner.TryDeliverHeroicInspirationReward();
                if (attacker->GetGameTag(GameTag::BACON_RALLY) != 0)
                    for (const auto& trinket : owner.season14.trinkets)
                        if (trinket.active && trinket.remainingUses != 0 &&
                            FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                                TrinketEffect::RALLY_ATTACK_FREE_REFRESH)
                            owner.season14.AddFreeRefreshes(1);
                // This is a real attack declaration, even though it is an
                // immediate start-of-combat edge attack rather than the main
                // attack loop.  Faerie Dragon Scale's per-combat trigger
                // must see it as well.
                for (auto& trinket : owner.season14.trinkets)
                {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    const auto behavior = FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id);
                    if (behavior.effect == TrinketEffect::START_COMBAT_DRAGON_SHIELDS &&
                        attacker->HasRace(Race::DRAGON) &&
                        trinket.triggerProgress < behavior.value)
                    {
                        attacker->SetGameTag(GameTag::DIVINE_SHIELD, 1);
                        ++trinket.triggerProgress;
                    }
                }
                target.TakeDamage(*attacker);
                attacker->TakeDamage(target);
                if (owner.season14.HasGeneratedRewardVolatileVenom())
                    attacker->SetHealth(0);
            }
            // This deferred Rally attack is still an attack resolution.  Keep
            // its attacker identity alive through death processing so effects
            // such as Mutalisk are credited to the Rally owner's field.
            m_killContext = KillContext{
                attacker->GetIndex(),
                turn == Turn::PLAYER1 ? 1 : 2,
                true};
            ProcessDestroy(false);
            m_killContext = {};
        }
    };
    resolveYoHoOgre(m_player1, m_p1Field, m_p2Field, Turn::PLAYER1);
    resolveYoHoOgre(m_player2, m_p2Field, m_p1Field, Turn::PLAYER2);

    // Righteous Charge is a generated reward rather than a card task.  Its
    // leftmost combat copy gains Divine Shield and performs one attack before
    // the ordinary combat cursor begins.  Resolve through the normal attack
    // damage/death path so triggers and replay observe the attack.
    const auto resolveRighteousCharge = [this](Player& owner, FieldZone& own,
                                                FieldZone& enemy, Turn turn) {
        if (!owner.season14.HasGeneratedRewardRighteousCharge() || own.IsEmpty() ||
            !HasAttackableTarget(enemy)) return;
        Minion& attacker = own[0];
        m_turn = turn;
        auto& target = GetProperTarget(attacker);
        {
            AttackingStateGuard attacking(attacker);
            owner.season14.OnFriendlyMinionAttack();
            target.TakeDamage(attacker);
            attacker.TakeDamage(target);
        }
        m_killContext = KillContext{attacker.GetIndex(),
                                    turn == Turn::PLAYER1 ? 1 : 2, true};
        ProcessDestroy(false);
        m_killContext = {};
    };
    resolveRighteousCharge(m_player1, m_p1Field, m_p2Field, Turn::PLAYER1);
    resolveRighteousCharge(m_player2, m_p2Field, m_p1Field, Turn::PLAYER2);

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
    auto& attackerOwner = (m_turn == Turn::PLAYER1) ? m_player1 : m_player2;
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
    if (attacker.HasRace(Race::PIRATE))
            attackerOwner.season14.OnFriendlyPirateAttack();
    attackerOwner.season14.OnFriendlyMinionAttack();
    if (attacker.HasRace(Race::BEAST))
    {
        for (const auto& trinket : attackerOwner.season14.trinkets)
        {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(trinket.dbfID).id);
            if (behavior.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH)
            {
                attacker.SetAttack(attacker.GetAttack() + 2);
                attacker.SetHealth(attacker.GetHealth() + 2);
            }
        }
    }
    attackerOwner.TryDeliverHeroicInspirationReward();
    if (attacker.GetGameTag(GameTag::BACON_RALLY) != 0)
        for (const auto& trinket : attackerOwner.season14.trinkets)
            if (trinket.active && trinket.remainingUses != 0 &&
                FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                    TrinketEffect::RALLY_ATTACK_FREE_REFRESH)
                attackerOwner.season14.AddFreeRefreshes(1);
    for (auto& trinket : attackerOwner.season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::START_COMBAT_DRAGON_SHIELDS &&
            attacker.HasRace(Race::DRAGON) &&
            trinket.triggerProgress < behavior.value)
        {
            attacker.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            ++trinket.triggerProgress;
        }
        if (behavior.effect == TrinketEffect::ATTACKING_DRAGON_DIVINE_SHIELD &&
            attacker.HasRace(Race::DRAGON) &&
            trinket.triggerProgress < behavior.value)
        {
            attacker.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            ++trinket.triggerProgress;
        }
    }
            for (auto& trinket : attackerOwner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect == TrinketEffect::AFTER_TWO_ATTACKS_QUILBOAR_GEM &&
                    ++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    attackerOwner.battleField.ForEachAlive([&](MinionData& data) {
                        if (data.value().HasRace(Race::QUILBOAR)) data.value().ApplyBloodGem(1, 1);
                    });
                }
            }
    for (auto& trinket : attackerOwner.season14.trinkets)
    {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::ATTACKING_MINION_STATS)
            attacker.SetAttack(attacker.GetAttack() + behavior.attack);
        if (behavior.effect == TrinketEffect::ATTACKING_BEAST_SCALING &&
            attacker.HasRace(Race::BEAST))
        {
            attacker.SetAttack(attacker.GetAttack() + behavior.attack +
                               trinket.triggerProgress);
            ++trinket.triggerProgress;
        }
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
    // Lift Off's Advanced Ballistics is a player-owned Rally payload on the
    // hero's Battlecruiser.  It is not a static CardDef task because the
    // amount is selected by the bought upgrade tier.  Resolve it once at the
    // attack declaration boundary, excluding the Battlecruiser itself.
    if ((attacker.GetCardID() == "BG31_HERO_801pt" ||
         attacker.GetCardID() == "BG31_HERO_801pt_G") &&
        attackerOwner.season14.liftOffRallyAttack > 0)
    {
        const auto amount = attackerOwner.season14.liftOffRallyAttack;
        attackerField.ForEachAlive([&attacker, amount](MinionData& data) {
            if (data.value().GetIndex() != attacker.GetIndex())
                data.value().SetAttack(data.value().GetAttack() + amount);
        });
    }
    // Jailbird Juggernaut's Rally queues a Blood Gem Golem to attack the
    // already-selected target first. Resolve it here, after all Rally tasks
    // have run, so inserting the Golem cannot invalidate the active attacker
    // or target references during task dispatch.
    for (const auto& pending : attackerOwner.season14.TakeBloodGemGolemAttacks()) {
        const Card golemCard = Cards::FindCardByID("BG30_MagicItem_442t");
        if (!golemCard.id.empty() && attackerField.IsFull()) {
            attackerOwner.ApplySummonOverflowTrinkets();
        } else if (!golemCard.id.empty()) {
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
            attackerOwner.ApplySummonTrinkets(summoned);
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
                m_killContext = KillContext{
                    summoned.GetIndex(),
                    m_turn == Turn::PLAYER1 ? 1 : 2,
                    true};
                ProcessDestroy(false);
                m_killContext = {};
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
    const int targetHealthBeforeAttack = nextTarget.GetHealth();
    const bool attackedFriendlyTaunt = nextTarget.HasTaunt();
    // Blade Collector's attack is a cleave: damage adjacent enemies of the
    // selected target before the ordinary exchange, so all damage participates
    // in the same subsequent death/trigger cleanup.
    const bool bladeCollector = attackerAfterRally.GetCardID() == "BG26_817" ||
                                attackerAfterRally.GetCardID() == "BG26_817_G";
    const bool ultralisk = attackerAfterRally.GetCardID() == "BG31_HERO_811t10" ||
                           attackerAfterRally.GetCardID() == "BG31_HERO_811t10_G";
    bool eclipsionImmune = false;
    const bool attackerAlreadyImmune = attackerAfterRally.HasImmuneWhileAttacking();
    int* eclipsionAttacks = (m_turn == Turn::PLAYER1)
                                ? &m_p1EclipsionAttacks
                                : &m_p2EclipsionAttacks;
    int eclipsionLimit = 0;
    attackerField.ForEachAlive([&eclipsionLimit](const MinionData& data) {
        const auto& id = data.value().GetCardID();
        const auto lifecycle = CardDefs::FindCardDefByID(id).lifecycle;
        if (lifecycle == CardLifecycle::BUDDY_ECLIPSION)
            // Each owned Buddy contributes its own "first" attack allowance;
            // multiple copies therefore stack (normal +1, golden +2).
            eclipsionLimit += id.ends_with("_G") ? 2 : 1;
    });
    if (*eclipsionAttacks < eclipsionLimit) {
        attackerAfterRally.SetImmuneWhileAttacking(true);
        ++*eclipsionAttacks;
        eclipsionImmune = true;
    }
    if (bladeCollector || ultralisk) {
        const int position = nextTarget.GetZonePosition();
        std::vector<Minion*> adjacent;
        defendingField.ForEachAlive([&](MinionData& data) {
            const int candidate = data.value().GetZonePosition();
            if (candidate == position - 1 || candidate == position + 1)
                adjacent.push_back(&data.value());
        });
        for (auto* adjacentTarget : adjacent)
            adjacentTarget->TakeDamage(attackerAfterRally.GetAttack());
    }
    {
        AttackingStateGuard attacking(attackerAfterRally);
        nextTarget.TakeDamage(attackerAfterRally);
        attackerAfterRally.TakeDamage(nextTarget);
        if (attackerOwner.season14.HasGeneratedRewardVolatileVenom())
            attackerAfterRally.SetHealth(0);
    }
    // Eclipsion's immunity is a one-attack grant. Preserve any independent
    // permanent immunity, but consume this Buddy-provided flag immediately
    // after the exchange so Windfury/next-turn attacks are not protected.
    if (eclipsionImmune && !attackerAlreadyImmune)
        attackerAfterRally.SetImmuneWhileAttacking(false);
    // Wandering Treant observes the attack event, not the summon event.  Take
    // a snapshot before cleanup so a Taunt that dies in this exchange still
    // satisfies the trigger; only surviving friendly minions receive the
    // permanent gain.  Each Treant copy contributes independently.
    if (attackedFriendlyTaunt) {
        int treantAttack = 0;
        int treantHealth = 0;
        attackerField.ForEachAlive([&](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "TB_BaconShop_HERO_95_Buddy") {
                treantAttack += 1;
                treantHealth += 1;
            } else if (id == "TB_BaconShop_HERO_95_Buddy_G") {
                treantAttack += 2;
                treantHealth += 2;
            }
        });
        if (treantAttack != 0)
            attackerField.ForEachAlive([treantAttack, treantHealth](MinionData& data) {
                data.value().ApplyPersistentMinionStats(treantAttack, treantHealth);
            });
    }
    if (eclipsionImmune)
        attackerAfterRally.SetImmuneWhileAttacking(attackerAlreadyImmune);
    const bool targetWasDestroyed = nextTarget.IsDestroyed();
    // Capture the killed entity's catalogue identity before cleanup can
    // invalidate/compact the combat field.  Loyal Henchman copies this plain
    // source after the confirmed attack kill, not arbitrary death events.
    const std::string killedCardID = targetWasDestroyed
        ? std::string(nextTarget.GetCardID()) : std::string{};

    // Wildfire Elemental carries excess combat damage into one adjacent
    // enemy. Resolve it before cleanup while the defeated target still has a
    // stable zone position; the adjacent hit itself participates in the
    // normal damage/death lifecycle below.
    const bool wildfire = attackerAfterRally.GetCardID() == "BGS_126" ||
                          attackerAfterRally.GetCardID() == "TB_BaconUps_166";
    if (targetWasDestroyed && wildfire) {
        const int excess = std::max(0, attackerAfterRally.GetAttack() - targetHealthBeforeAttack);
        if (excess > 0) {
            std::vector<Minion*> adjacent;
            const int position = nextTarget.GetZonePosition();
            defendingField.ForEachAlive([&](MinionData& data) {
                const int candidate = data.value().GetZonePosition();
                if (candidate == position - 1 || candidate == position + 1)
                    adjacent.push_back(&data.value());
            });
            if (attackerAfterRally.GetCardID() == "TB_BaconUps_166")
                for (auto* target : adjacent) target->TakeDamage(excess);
            else if (!adjacent.empty())
                adjacent[Random::get<std::size_t>(0, adjacent.size() - 1)]->TakeDamage(excess);
        }
    }

    m_killContext = KillContext{attackerAfterRally.GetIndex(),
                                (m_turn == Turn::PLAYER1) ? 1 : 2, true};
    ProcessDestroy(false);
    m_killContext = {};

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
    // Captain Fairmount improves Conviction after a friendly combat kill.
    // Read the combat field: a Buddy that died earlier in this combat no
    // longer supplies its trigger. Queue the modal for post-combat so no
    // player decision is requested in the middle of combat resolution.
    if (targetWasDestroyed) {
        int improvements = 0;
        attackerField.ForEachAlive([&improvements](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG21_HERO_000_Buddy") improvements += 1;
            else if (id == "BG21_HERO_000_Buddy_G") improvements += 2;
        });
        auto& owner = m_turn == Turn::PLAYER1 ? m_player1 : m_player2;
        // The attacker-side field is authoritative for ownership: only a
        // confirmed enemy death during attack resolution emits this event.
        // Deathrattles and simultaneous cleanup therefore cannot fabricate a
        // Sulfuras kill, and a killer dying in the same exchange still counts.
        (void)owner.season14.RecordFriendlyCombatKill();
        if (owner.season14.heroPowerDbfID == 73941 && improvements > 0)
            owner.season14.QueueConvictionImprovements(improvements);
        // Icesnarl's printed +1/+2 Health is permanent and must be applied at
        // the confirmed kill boundary, so later attacks in this same combat
        // see the increased health.  Apply per instance: multiple Buddies do
        // not pool their scaling, and a dead Buddy cannot trigger retroactively.
        attackerField.ForEachAlive([](MinionData& data) {
            auto& buddy = data.value();
            const auto& id = buddy.GetCardID();
            const int health = id == "BG20_HERO_100_Buddy_G" ? 2 :
                               id == "BG20_HERO_100_Buddy" ? 1 : 0;
            if (health > 0)
                buddy.ApplyCombatPersistentStats(0, health);
        });
        if (!killedCardID.empty()) {
            const auto killedCard = Cards::FindCardByID(killedCardID);
            if (!killedCard.id.empty()) {
                Minion killedSnapshot(killedCard);
                owner.ResolveLoyalHenchmanKill(killedSnapshot);
            }
        }
    }

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

void Battle::TryFireQueuedLockAndLoad()
{
    if (m_lockAndLoadResolving) return;
    m_lockAndLoadResolving = true;
    const auto fire = [this](Player& owner, FieldZone& own, FieldZone& enemy) {
        if (own.IsFull() || !HasAttackableTarget(enemy)) return;
        const auto before = own.GetCount();
        owner.ResolveLockAndLoad();
        if (own.GetCount() <= before) return;
        std::vector<Minion*> targets;
        enemy.ForEachAlive([&targets](MinionData& data) {
        if (!data.value().HasStealth()) targets.push_back(&data.value());
        });
        if (targets.empty()) return;
        auto& projectile = own[own.GetCount() - 1];
        auto& target = *targets[Random::get<std::size_t>(0, targets.size() - 1)];
        AttackingStateGuard attacking(projectile);
        target.TakeDamage(projectile);
        projectile.TakeDamage(target);
    };
    fire(m_player1, m_p1Field, m_p2Field);
    fire(m_player2, m_p2Field, m_p1Field);
    m_lockAndLoadResolving = false;
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
        // Radio Star's deathrattle needs the exact attacking enemy instance,
        // including current health and enchantments.  Capture it before any
        // DEATH trigger or zone removal can invalidate the attacker snapshot.
        if (!beforeAttack &&
            (minion.GetCardID() == "BG34_Giant_330" ||
             minion.GetCardID() == "BG34_Giant_330_G") &&
            minion.LastDamageSourceIndex() >= 0)
        {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            Player& enemy = std::get<0>(deadMinion) == 1 ? m_player2 : m_player1;
            const auto index = static_cast<std::size_t>(minion.LastDamageSourceIndex());
            if (index < enemy.battleField.GetCount())
                owner.season14.ArmExactCopyDeathrattle(enemy.battleField[index]);
        }
        // Keep a plain snapshot of every friendly combat death.  Consumers
        // select their own race/filter (Kangor still asks for MECHANICAL),
        // while generated rewards such as Turbulent Tombs and Victim's
        // Specter must also see non-Mechanical minions.
        (std::get<0>(deadMinion) == 1 ? m_player1 : m_player2)
            .season14.RecordCombatDeadMinion(minion);
        Minion removedMinion;
        if (std::get<0>(deadMinion) == 1)
            m_player1.season14.RecordReclaimedSoulsDeath(minion);
        else
            m_player2.season14.RecordReclaimedSoulsDeath(minion);
        (std::get<0>(deadMinion) == 1 ? m_player1 : m_player2)
            .ResolveMechGyverDeath();

        // Shadowy Construct snapshots the dead minion's maximum stats before
        // it leaves the combat board. Each normal copy triggers once and each
        // golden copy twice per combat; key counters by entity so multiple
        // copies do not incorrectly share a single allowance.
        const auto ownerSide = std::get<0>(deadMinion);
        Player& owner = ownerSide == 1 ? m_player1 : m_player2;
        owner.battleField.ForEachAlive([&](MinionData& data) {
            auto& construct = data.value();
            const auto& id = construct.GetCardID();
            if (id != "BG25_HERO_103_Buddy" &&
                id != "BG25_HERO_103_Buddy_G") return;
            const int limit = id.ends_with("_G") ? 2 : 1;
            // Combat-copy indices are normally globally allocated, but the
            // bridge contract does not require that across seats.  Namespace
            // the entity key by owner so two copies with the same local
            // index cannot consume one another's once/twice-per-combat use.
            const auto entityKey = (static_cast<std::uint64_t>(ownerSide) << 32) |
                                   static_cast<std::uint32_t>(construct.GetIndex());
            auto& uses = m_shadowyConstructTriggers[entityKey];
            if (uses >= limit) return;
            construct.SetAttack(construct.GetAttack() + minion.GetAttack());
            construct.SetHealth(construct.GetHealth() + minion.GetMaxHealth());
            ++uses;
        });

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

            // Scrapsmith Portrait is owner-scoped: only the owner of the
            // friendly Taunt death receives a permanent Gem on Scrapsmiths.
            if (minion.HasTaunt()) owner.ResolveScrapsmithPortraitDeath(minion);

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

            if (minion.HasTaunt()) owner.ResolveScrapsmithPortraitDeath(minion);

            minion.SetLastFieldPos(minion.GetZonePosition());
            removedMinion = m_p2Field.Remove(minion);
        }

        // Boom Controller snapshots the first friendly Mech only after the
        // authoritative removal, but before any later deathrattle mutates the
        // instance.  Its copy is resolved at the completed death boundary.
        if (removedMinion.HasRace(Race::MECHANICAL)) {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            const bool hasBoomController = std::any_of(
                owner.season14.trinkets.begin(), owner.season14.trinkets.end(),
                [](const Season14PersistentEffect& trinket) {
                    return trinket.active && trinket.remainingUses != 0 &&
                           trinket.triggerProgress == 0 &&
                           FindTrinketBehavior(
                               Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                               TrinketEffect::BOOM_CONTROLLER_FIRST_MECH_COPY;
                });
            if (hasBoomController)
                owner.season14.RecordBoomControllerMech(removedMinion);
        }

        // Elementium Squirrel Bomb resolves after its source is removed. The
        // combat-death snapshot already includes the bomb itself, so its
        // printed "each Mech that died" wording naturally counts all
        // friendly Mechs seen so far, including this death. Patch 36.4
        // prints four damage (eight golden) per packet, while each packet
        // independently chooses a
        // currently live enemy target.
        if (CardDefs::FindCardDefByID(removedMinion.GetCardID()).lifecycle ==
            CardLifecycle::BUDDY_ELEMENTIUM_SQUIRREL_BOMB) {
            const int packetDamage = removedMinion.GetCardID().ends_with("_G") ? 8 : 4;
            const auto mechDeaths = owner.season14.CountCombatDeadMinions(Race::MECHANICAL);
            FieldZone& enemyField = &owner == &m_player1 ? m_p2Field : m_p1Field;
            for (std::size_t packet = 0; packet < mechDeaths; ++packet) {
                std::vector<Minion*> targets;
                enemyField.ForEachAlive([&targets](MinionData& data) {
                    if (!data.value().IsDestroyed() && !data.value().HasStealth())
                        targets.push_back(&data.value());
                });
                if (targets.empty()) break;
                targets[Random::get<std::size_t>(0, targets.size() - 1)]->TakeDamage(packetDamage);
            }
        }
        // Spirit of Air gives random friendly minions all three printed
        // keywords after its death. Golden copies affect two distinct targets.
        if (removedMinion.GetCardID() == "TB_BaconShop_HERO_76_Buddy" ||
            removedMinion.GetCardID() == "TB_BaconShop_HERO_76_Buddy_G") {
            const int copies = removedMinion.GetCardID().ends_with("_G") ? 2 : 1;
            std::vector<Minion*> targets;
            owner.battleField.ForEachAlive([&targets](MinionData& data) {
                if (!data.value().IsDestroyed()) targets.push_back(&data.value());
            });
            for (int copy = 0; copy < copies && !targets.empty(); ++copy) {
                const auto index = Random::get<std::size_t>(0, targets.size() - 1);
                auto* target = targets[index];
                // This is a permanent deathrattle gain. Mark the combat copy
                // as persistent so Battle::CommitPersistentState transfers
                // exactly these keywords to the matching recruit entity;
                // mutating only the temporary copy would lose the effect at
                // the combat boundary.
                target->ApplyCombatPersistentKeyword(GameTag::WINDFURY);
                target->ApplyCombatPersistentKeyword(GameTag::DIVINE_SHIELD);
                target->ApplyCombatPersistentKeyword(GameTag::TAUNT);
                targets.erase(targets.begin() + static_cast<std::ptrdiff_t>(index));
            }
        }
        // Boom Squad counts every confirmed friendly combat death, including
        // tokens and minions without deathrattles. Resolve against the live
        // opposing combat field before subsequent cleanup can remove it.
        owner.ResolveGeneratedQuestRewardCombatDeath(
            &owner == &m_player1 ? m_p2Field : m_p1Field);
        // Mutalisk's combat kill observer: only confirmed enemy deaths during
        // an attack-resolution pass count; cleanup/deathrattle passes do not.
        if (m_killContext.KilledEnemy(std::get<0>(deadMinion))) {
            Player& killer = m_killContext.attackerOwner == 1 ? m_player1 : m_player2;
            // Tide Oracle Morgl triggers only for a confirmed attack kill,
            // not for incidental damage or a deathrattle.  Attribute the
            // effect to the exact attacking entity and copy the slain
            // minion's maximum stats into one friendly minion in hand.
            Minion* attacker = nullptr;
            killer.battleField.ForEachAlive([&](MinionData& data) {
                if (attacker == nullptr &&
                    data.value().GetIndex() == m_killContext.attackerEntityID)
                    attacker = &data.value();
            });
            if (attacker && (attacker->GetCardID() == "BG27_513" ||
                             attacker->GetCardID() == "BG27_513_G")) {
                const int multiplier = attacker->GetCardID() == "BG27_513_G" ? 2 : 1;
                bool applied = false;
                killer.hand.ForEach([&](std::optional<CardData>& entry) {
                    if (applied || !entry.has_value() ||
                        !std::holds_alternative<Minion>(*entry)) return;
                    auto& handMinion = std::get<Minion>(*entry);
                    handMinion.SetAttack(handMinion.GetAttack() +
                                         removedMinion.GetAttack() * multiplier);
                    handMinion.SetHealth(handMinion.GetHealth() +
                                         removedMinion.GetMaxHealth() * multiplier);
                    applied = true;
                });
            }
            killer.battleField.ForEachAlive([](MinionData& data) {
                auto& mutalisk = data.value();
                if (mutalisk.GetCardID() == "BG31_HERO_811t6")
                    mutalisk.ApplyCombatPersistentStats(3, 0);
                else if (mutalisk.GetCardID() == "BG31_HERO_811t6_G")
                    mutalisk.ApplyCombatPersistentStats(6, 0);
            });
        }
        // Rot Hide Gnoll counts every friendly minion death in this combat.
        // The battle snapshot is intentionally the only state mutated: the
        // recruit board is the next-turn baseline and must not retain this
        // combat-only attack bonus.
        owner.battleField.ForEachAlive([](MinionData& data) {
            auto& gnoll = data.value();
            if (gnoll.GetCardID() == "BG25_013")
                gnoll.SetAttack(gnoll.GetAttack() + 1);
            else if (gnoll.GetCardID() == "BG25_013_G")
                gnoll.SetAttack(gnoll.GetAttack() + 2);
        });
        if (removedMinion.GetCardID() == "BG25_008" ||
            removedMinion.GetCardID() == "BG25_008_G")
            ++owner.eternalKnightsDiedThisGame;
        // Eternal Knight is a dynamic wherever-this-is aura. Re-apply only
        // the newly earned delta to every surviving copy in both snapshots.
        if (removedMinion.GetCardID() == "BG25_008" ||
            removedMinion.GetCardID() == "BG25_008_G") {
            owner.battleField.ForEachAlive([&owner](MinionData& data) {
                data.value().ApplyEternalKnightDeathCount(
                    owner.eternalKnightsDiedThisGame);
            });
            owner.recruitField.ForEachAlive([&owner](MinionData& data) {
                data.value().ApplyEternalKnightDeathCount(
                    owner.eternalKnightsDiedThisGame);
            });
        }
        if (removedMinion.HasRace(Race::UNDEAD) &&
            owner.HasActivePortrait(PortraitEffect::ETERNAL_KNIGHT_UNDEAD_STATS)) {
            ++owner.undeadDiedThisGame;
            owner.battleField.ForEachAlive([&owner](MinionData& data) {
                data.value().ApplyEternalKnightUndeadDeathCount(
                    owner.undeadDiedThisGame);
            });
            owner.recruitField.ForEachAlive([&owner](MinionData& data) {
                data.value().ApplyEternalKnightUndeadDeathCount(
                    owner.undeadDiedThisGame);
            });
        }
        if (owner.season14.heroPowerDbfID == 66484 &&
            removedMinion.GetCardID() != "TB_BaconShop_HP_105t" &&
            removedMinion.GetCardID() != "TB_BaconShop_HP_105t_SKIN_A" &&
            removedMinion.GetCardID() != "TB_BaconShop_HP_105t_SKIN_A_G")
        {
            owner.battleField.ForEachAlive([&](MinionData& data) {
                if (data.value().GetCardID() == "TB_BaconShop_HP_105t" ||
                    data.value().GetCardID() == "TB_BaconShop_HP_105t_SKIN_A")
                    removedMinion.CopyDeathrattleTo(data.value());
            });
        }
        // Sneed's golden token has no reviewed child CardDef task and keeps
        // the bespoke two-roll fallback.  The normal starting Shredder has
        // BG21_HERO_030pe attached at hero-selection time; its generated
        // child task is authoritative and is allowed to run below, avoiding
        // a duplicate summon.
        if (removedMinion.GetCardID() == "BG21_HERO_030t" ||
            removedMinion.GetCardID() == "BG21_HERO_030t_G")
        {
            const bool hasReviewedChild = removedMinion.HasDeathrattle();
            if (!hasReviewedChild ||
                removedMinion.GetCardID() == "BG21_HERO_030t_G")
            owner.ResolveSneedShredderDeathrattle(
                removedMinion.GetCardID() == "BG21_HERO_030t_G");
        }

        // Process deathrattle tasks
        if (removedMinion.HasDeathrattle())
        {
            // BG24_Reward_113_ALT repeats the first friendly Deathrattle of
            // each combat. Claim the first-deathrattle slot before executing
            // any task: a source Deathrattle can synchronously kill another
            // minion, and that nested death must not recursively qualify for
            // Phylactery/Ritual Dagger/Tombs. The counter is reset at
            // COMBAT_START and remains a combat-local event guard.
            int repeatFirstDeathrattleCount = 0;
            const bool firstDeathrattle =
                owner.season14.deathrattlesTriggered == 0;
            if (firstDeathrattle)
                ++owner.season14.deathrattlesTriggered;
            if (owner.season14.HasGeneratedRewardRitualDaggerRepeat() ||
                owner.season14.HasGeneratedRewardTurbulentTombs())
                repeatFirstDeathrattleCount = 1;
            if (firstDeathrattle)
                for (const auto& trinket : owner.season14.trinkets)
                {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    const auto behavior = FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id);
                    if (behavior.effect == TrinketEffect::DEATHLY_PHYLACTERY)
                        ++repeatFirstDeathrattleCount;
                }
            // Thornspike Pauldron is keyed to each Deathrattle trigger, not
            // to the removed minion/death boundary.  In particular, a
            // repeated first Deathrattle is two triggers.  Arm the temporary
            // Blood Gem modifier immediately after each activation so any
            // later Deathrattle activation (or other generated Gem) observes
            // the modifier, while the activation that caused the trigger does
            // not retroactively change its already-resolved Gems.
            const auto armThornspike = [&owner]() {
                for (const auto& trinket : owner.season14.trinkets)
                {
                    if (!trinket.active || trinket.remainingUses == 0) continue;
                    const auto behavior = FindTrinketBehavior(
                        Cards::FindCardByDbfID(trinket.dbfID).id);
                    if (behavior.effect ==
                        TrinketEffect::AFTER_DEATHRATTLE_TEMP_BLOOD_GEM_BONUS)
                        owner.season14.AddTemporaryBloodGemBonus(behavior.attack, behavior.health);
                }
            };
            removedMinion.ActivateTask(
                PowerType::DEATHRATTLE,
                owner);
            armThornspike();
            for (int repeat = 0; repeat < repeatFirstDeathrattleCount; ++repeat)
            {
                removedMinion.ActivateTask(PowerType::DEATHRATTLE, owner);
                armThornspike();
            }
            // Unholy Sanctum resolves after the deathrattle and permanently
            // buffs the right-most surviving friendly minion.
            for (const auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AFTER_DEATHRATTLE_RIGHTMOST_STATS)
                    continue;
                Minion* rightmost = nullptr;
                owner.battleField.ForEachAlive([&rightmost](MinionData& data) {
                    rightmost = &data.value();
                });
                if (!rightmost) continue;
                rightmost->SetAttack(rightmost->GetAttack() + behavior.attack);
                rightmost->SetHealth(rightmost->GetHealth() + behavior.health);
                const auto index = rightmost->GetIndex();
                owner.recruitField.ForEachAlive([&](MinionData& data) {
                    auto& recruit = data.value();
                    if (recruit.GetIndex() == index)
                    {
                        recruit.SetAttack(recruit.GetAttack() + behavior.attack);
                        recruit.SetHealth(recruit.GetHealth() + behavior.health);
                    }
                });
            }
            owner.ResolveGeneratedQuestRewardDeath(removedMinion);
            // Blood Amulet plays three permanent Blood Gems on distinct
            // random friendly minions after the Deathrattle boundary.  Build
            // the candidate list after the Deathrattle so summons/removals
            // are reflected, and consume one owned instance independently.
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AFTER_DEATHRATTLE_BLOOD_GEMS)
                    continue;
                std::vector<Minion*> candidates;
                owner.battleField.ForEachAlive([&candidates](MinionData& data) {
                    candidates.push_back(&data.value());
                });
                const auto count = std::min<int>(behavior.amount,
                                                 static_cast<int>(candidates.size()));
                for (int i = 0; i < count; ++i)
                {
                    const auto index = Random::get(0, static_cast<int>(candidates.size()) - 1);
                    owner.ApplyBloodGemTo(*candidates[static_cast<std::size_t>(index)]);
                    candidates.erase(candidates.begin() + index);
                }
            }
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect == TrinketEffect::AVENGE_RANDOM_MAGNETIC &&
                    ++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    (void)SimpleTasks::RandomCardToHandTask{Race::INVALID, 0, 1, true}.Run(owner);
                }
            }
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0 || trinket.triggerProgress != 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::FIRST_DEATH_MAX_STATS_RANDOM) continue;
                std::vector<Minion*> targets;
                owner.battleField.ForEachAlive([&](MinionData& data) { targets.push_back(&data.value()); });
                if (!targets.empty())
                {
                    const auto recipientCount = std::max(1, behavior.amount);
                    for (int recipient = 0;
                         recipient < recipientCount && !targets.empty();
                         ++recipient)
                    {
                        const auto index = Random::get<std::size_t>(0, targets.size() - 1);
                        auto* target = targets[index];
                        target->SetAttack(target->GetAttack() + removedMinion.GetAttack());
                        target->SetHealth(target->GetHealth() + removedMinion.GetHealth());
                        // Golden Alliance Keychain selects distinct friendly
                        // recipients when the warband has enough targets.
                        targets.erase(targets.begin() + index);
                    }
                    trinket.triggerProgress = 1;
                }
            }
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::AVENGE_RANDOM_UNDEAD_REBORN ||
                    ++trinket.triggerProgress < behavior.value) continue;
                trinket.triggerProgress = 0;
                std::vector<Minion*> undead;
                owner.battleField.ForEachAlive([&](MinionData& data) {
                    if (data.value().HasRace(Race::UNDEAD) && !data.value().HasReborn()) undead.push_back(&data.value());
                });
                if (!undead.empty()) undead[Random::get<std::size_t>(0, undead.size() - 1)]->SetReborn(true);
            }
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect == TrinketEffect::AVENGE_BLOOD_GEM_BONUS &&
                    ++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    owner.season14.AddBloodGemBonus(behavior.attack, behavior.health);
                }
            }
            owner.UpdateSkyGolemsForDeathrattle();
            owner.AdvanceDarkGiftCounters(2);
        }

        // Powder Keg is deliberately resolved after the source's ordinary
        // Deathrattle chain.  Its marker is combat-copy state, so a dead
        // Pirate's current Attack is preserved even when the recruit copy
        // has a different persistent value.  The Sky Pirate is inserted
        // through the normal summon lifecycle and immediately attacks via
        // the same damage/death boundary as other generated attackers.
        if (removedMinion.PowderKegDeathrattleAttack() > 0)
        {
            FieldZone& ownerField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            FieldZone& enemyField = std::get<0>(deadMinion) == 1 ? m_p2Field : m_p1Field;
            const Turn previousTurn = m_turn;
            // A death pass can process the defending side first.  Immediate
            // attacks must nevertheless resolve from the summoned Pirate's
            // side, so target selection and attack-side hooks use ownerField.
            const Turn ownerTurn = std::get<0>(deadMinion) == 1
                                       ? Turn::PLAYER1
                                       : Turn::PLAYER2;
            const int attack = removedMinion.PowderKegDeathrattleAttack();
            removedMinion.SetPowderKegDeathrattleAttack(0);
            if (!ownerField.IsFull())
            {
                Player& sourceOwner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
                const Card token = Cards::FindCardByID("BGS_061t");
                if (!token.id.empty())
                {
                    Minion pirate{token};
                    pirate.SetAttack(attack);
                    pirate.SetHealth(1);
                    sourceOwner.ApplyFreshMinionModifiers(pirate);
                    pirate.getPlayerCallback = [&sourceOwner]() -> Player& { return sourceOwner; };
                    if (sourceOwner.getNextCardIndexCallback)
                        pirate.SetIndex(sourceOwner.getNextCardIndexCallback());
                    ownerField.Add(pirate, ownerField.GetCount());
                    Minion& summoned = ownerField[ownerField.GetCount() - 1];
                    ownerField.ForEachAlive([&summoned](MinionData& data) {
                        data.value().ActivateTrigger(TriggerType::SUMMON, summoned);
                    });
                    sourceOwner.ApplySummonTrinkets(summoned);
                    if (HasAttackableTarget(enemyField))
                    {
                        m_turn = ownerTurn;
                        Minion& target = GetProperTarget(summoned);
                        AttackingStateGuard attacking(summoned);
                        if (summoned.HasRace(Race::PIRATE))
                            sourceOwner.season14.OnFriendlyPirateAttack();
                        sourceOwner.season14.OnFriendlyMinionAttack();
                        target.TakeDamage(summoned);
                        summoned.TakeDamage(target);
                        m_killContext = KillContext{
                            summoned.GetIndex(),
                            ownerTurn == Turn::PLAYER1 ? 1 : 2,
                            true};
                        ProcessDestroy(false);
                        m_killContext = {};
                    }
                }
            }
            else
            {
                Player& sourceOwner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
                sourceOwner.ApplySummonOverflowTrinkets();
            }
            m_turn = previousTurn;
        }

        // Sr. Tomb Diver resolves after the deathrattle event has selected
        // and removed its source, so the live board order is authoritative.
        // The golden form upgrades the two right-most survivors.
        for (const auto& definition : BUDDY_RIGHTMOST_GOLDENIZE_BEHAVIORS) {
            if (removedMinion.GetCardID() != definition.id) continue;
            std::vector<Minion*> survivors;
            owner.battleField.ForEachAlive([&survivors](MinionData& data) {
                // "Right-most" is restricted to minions the effect can
                // actually convert. A pre-existing Golden minion must not
                // consume one of the normal/golden Buddy's target slots and
                // thereby prevent an eligible minion further left from being
                // selected.
                if (data.value().CanMakeGolden())
                    survivors.push_back(&data.value());
            });
            for (int i = 0; i < definition.targets && !survivors.empty(); ++i) {
                Minion* target = survivors.back();
                survivors.pop_back();
                target->MakeGolden();
            }
            break;
        }

        // Fish of N'Zoth gains the just-resolved friendly deathrattle twice.
        // Copy only after the source task chain completes; this preserves
        // ordering and prevents a copied task from seeing a half-resolved
        // source state.
        if (removedMinion.HasDeathrattle() &&
            removedMinion.GetCardID() != "TB_BaconUps_307") {
            owner.battleField.ForEachAlive([&removedMinion](MinionData& data) {
                auto& fish = data.value();
                if (fish.GetCardID() == "TB_BaconUps_307") {
                    removedMinion.CopyDeathrattleTo(fish);
                    removedMinion.CopyDeathrattleTo(fish);
                }
            });
        }

        // Avenge counts every friendly death, not only deaths that happen to
        // have a Deathrattle. The Deathrattle branch above handles those
        // deaths after their Deathrattle resolves; handle plain minion deaths
        // here without double-counting.
        if (!removedMinion.HasDeathrattle())
        {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect == TrinketEffect::AVENGE_RANDOM_UNDEAD_REBORN &&
                    ++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    std::vector<Minion*> undead;
                    owner.battleField.ForEachAlive([&](MinionData& data) {
                        if (data.value().HasRace(Race::UNDEAD) && !data.value().HasReborn()) undead.push_back(&data.value());
                    });
                    if (!undead.empty()) undead[Random::get<std::size_t>(0, undead.size() - 1)]->SetReborn(true);
                }
                if (behavior.effect == TrinketEffect::AVENGE_BLOOD_GEM_BONUS &&
                    ++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    owner.season14.AddBloodGemBonus(behavior.attack, behavior.health);
                }
            }
        }

        // These Trinkets count every friendly combat death, regardless of
        // whether the dead minion had a Deathrattle.  Resolve after the
        // minion's own deathrattle path so the generated hand card sees the
        // same authoritative event ordering as other death counters.
        {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            for (auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect ==
                    TrinketEffect::AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL)
                {
                    if (removedMinion.GetRace() == Race::INVALID)
                        (void)SimpleTasks::RandomTavernSpellToHandTask{1}.Run(owner);
                    continue;
                }
                if (behavior.effect != TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION ||
                    behavior.value <= 0) continue;
                if (++trinket.triggerProgress >= behavior.value)
                {
                    trinket.triggerProgress = 0;
                    (void)SimpleTasks::RandomCardToHandTask{
                        behavior.race, behavior.tier, 1,
                        behavior.magneticOnly, behavior.battlecryOnly}.Run(owner);
                }
            }
        }

        // Blood Golem Sticker watches every friendly Quilboar death,
        // including Quilboars summoned after combat began. Resolve it after
        // the minion's own deathrattle so the newly freed slot is available.
        if (removedMinion.HasRace(Race::QUILBOAR))
        {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            bool hasBloodGolemSticker = false;
            for (const auto& trinket : owner.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                if (FindTrinketBehavior(Cards::FindCardByDbfID(trinket.dbfID).id).effect ==
                    TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GOLEM)
                {
                    hasBloodGolemSticker = true;
                    break;
                }
            }
            if (hasBloodGolemSticker)
                SimpleTasks::QuilboarBloodGolemDeathrattleTask{}.Run(owner, removedMinion);
        }

        if (removedMinion.DeathrattleAttackTransfer() != 0 ||
            removedMinion.DeathrattleHealthTransfer() != 0)
        {
            FieldZone& ownerField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            bool transferred = false;
            ownerField.ForEachAlive([&](MinionData& data) {
                if (transferred && !removedMinion.DeathrattleStatTransferToAll()) return;
                auto& target = data.value();
                target.SetAttack(target.GetAttack() + removedMinion.DeathrattleAttackTransfer());
                target.SetHealth(target.GetHealth() + removedMinion.DeathrattleHealthTransfer());
                transferred = true;
            });
            removedMinion.SetDeathrattleStatTransfer(0, 0);
            removedMinion.SetDeathrattleStatTransferToAll(false);
        }

        if (removedMinion.HasEarthElementalDeathrattle())
        {
            FieldZone& ownerField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            if (ownerField.IsFull()) {
                owner.ApplySummonOverflowTrinkets();
            } else
            {
                const Card token = Cards::FindCardByDbfID(79728);
                if (!token.id.empty())
                {
                    Minion elemental(token);
                    elemental.getPlayerCallback = [&owner]() -> Player& { return owner; };
                    if (owner.getNextCardIndexCallback)
                        elemental.SetIndex(owner.getNextCardIndexCallback());
                    ownerField.Add(elemental, ownerField.GetCount());
                    Minion& summoned = ownerField[ownerField.GetCount() - 1];
                    ownerField.ForEachAlive([&summoned](MinionData& data) {
                        data.value().ActivateTrigger(TriggerType::SUMMON, summoned);
                    });
                    owner.ApplySummonTrinkets(summoned);
                }
            }
            removedMinion.SetEarthElementalDeathrattle(false);
        }

        if (removedMinion.GetCardID() == "BG22_HERO_001_Buddy" ||
            removedMinion.GetCardID() == "BG22_HERO_001_Buddy_G") {
            Player& owner = std::get<0>(deadMinion) == 1 ? m_player1 : m_player2;
            FieldZone& ownerField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
            FieldZone& enemyField = std::get<0>(deadMinion) == 1 ? m_p2Field : m_p1Field;
            const auto key = static_cast<std::uint64_t>(removedMinion.GetIndex());
            const auto it = std::find_if(owner.season14.spiritRaptorElements.begin(),
                                         owner.season14.spiritRaptorElements.end(),
                                         [key](const auto& entry) { return entry.first == key; });
            const auto definition = std::find_if(
                BUDDY_ELEMENT_MEMORY_BEHAVIORS.begin(),
                BUDDY_ELEMENT_MEMORY_BEHAVIORS.end(),
                [&](const auto& candidate) {
                    return candidate.id == removedMinion.GetCardID();
                });
            const int replay = definition == BUDDY_ELEMENT_MEMORY_BEHAVIORS.end()
                                   ? 0 : definition->replayCount;
            if (it != owner.season14.spiritRaptorElements.end()) {
                for (const auto element : it->second)
                    for (int n = 0; n < replay; ++n)
                        ApplyEmbraceElement(owner, ownerField, enemyField, element);
                owner.season14.spiritRaptorElements.erase(it);
            }
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
            if (ownerField.IsFull()) {
                owner.ApplySummonOverflowTrinkets();
            } else
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
                owner.ApplySummonTrinkets(removedMinion);
                // Dispatch only after the revived entity is inserted and
                // alive. This preserves deathrattle -> Reborn -> summon
                // ordering and gives post-Reborn effects the real instance.
                ownerField.ForEachAlive([&removedMinion](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::REBORN,
                                                  removedMinion);
                });
                owner.ApplyAfterRebornTrinkets(&removedMinion);
            }
        }

        // Lead the Frostwolves/Stormpikes use the same deterministic Avenge
        // lifecycle.  Resolve it only after deathrattle and Reborn handling so
        // a newly reborn friendly minion also receives the permanent bonus.
        auto& combatField = std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field;
        bool hasBuddyAvenge = false;
        combatField.ForEachAlive([&hasBuddyAvenge](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            hasBuddyAvenge = hasBuddyAvenge || id == "BG22_HERO_002_Buddy" ||
                             id == "BG22_HERO_002_Buddy_G" ||
                             id == "BG22_HERO_003_Buddy" ||
                             id == "BG22_HERO_003_Buddy_G";
        });
        if (hasBuddyAvenge && ++owner.season14.buddyAvengeDeaths >= 2) {
            int attack = 0, health = 0;
            combatField.ForEachAlive([&attack, &health](const MinionData& data) {
                const auto& id = data.value().GetCardID();
                if (id == "BG22_HERO_002_Buddy") attack += 1;
                else if (id == "BG22_HERO_002_Buddy_G") attack += 2;
                else if (id == "BG22_HERO_003_Buddy") health += 1;
                else if (id == "BG22_HERO_003_Buddy_G") health += 2;
            });
            owner.season14.buddyAvengeDeaths = 0;
            ApplyPermanentAvengeBonus(owner, combatField, attack, health);
        }
        // Monstrosity gains the dead friendly minion's Attack permanently;
        // golden Monstrosity gains it twice. Resolve on the combat copy so
        // the normal persistent-state reconciliation carries it back to the
        // recruit entity after combat.
        const int deadAttack = removedMinion.GetAttack();
        if (deadAttack > 0) {
            combatField.ForEachAlive([deadAttack](MinionData& data) {
                auto& receiver = data.value();
                if (receiver.GetCardID() == "BG20_HERO_282_Buddy")
                    receiver.SetAttack(receiver.GetAttack() + deadAttack);
                else if (receiver.GetCardID() == "BG20_HERO_282_Buddy_G")
                    receiver.SetAttack(receiver.GetAttack() + 2 * deadAttack);
            });
        }
        combatField.ForEachAlive([&owner](MinionData& data) {
            data.value().TriggerAvenge(owner);
        });
        if (owner.season14.AdvanceGeneratedRewardTumblingAvenge()) {
            // The permanent improvement is applied to future combat summons;
            // the current summon cycle has already resolved above.
        }
        const auto avenger = owner.season14.OnFriendlyMinionDiedBatch4();
        if (avenger.avengeTriggered)
        {
            ApplyPermanentAvengeBonus(
                owner,
                std::get<0>(deadMinion) == 1 ? m_p1Field : m_p2Field,
                avenger.attack, avenger.health);
        }
        if (owner.season14.heroPowerDbfID == 82114 &&
            owner.season14.AdvanceBroodmotherAvenge())
        {
            const auto token = Cards::FindCardByDbfID(82117);
            Minion whelp(token);
            const int whelpStats = 1 + owner.season14.broodmotherWhelpBonus;
            whelp.SetAttack(whelpStats);
            whelp.SetHealth(whelpStats);
            if (!token.id.empty() && owner.SummonCombatSnapshot(std::move(whelp))) {
                auto& enemy = (&combatField == &m_p1Field) ? m_p2Field : m_p1Field;
                if (HasAttackableTarget(enemy)) {
                    Minion& summoned = combatField[combatField.GetCount() - 1];
                    Minion& target = GetProperTarget(summoned);
                    AttackingStateGuard attacking(summoned);
                    target.TakeDamage(summoned);
                    summoned.TakeDamage(target);
                    ProcessDestroy(false);
                }
            }
            owner.season14.ImproveBroodmotherWhelp();
        }
        const auto trinketAvenger = owner.season14.OnTrinketFriendlyMinionDied();
        if (trinketAvenger.attack != 0 || trinketAvenger.health != 0 ||
            trinketAvenger.summonBeetles > 0 ||
            trinketAvenger.transferRightmostAttackToDragon ||
            trinketAvenger.triggerFriendlyBattlecry > 0)
        {
            ApplyPermanentAvengeBonus(
                owner, combatField, trinketAvenger.attack,
                trinketAvenger.health);
            // Gilnean Thorned Rose deals one damage to the same friendly
            // minions after granting the permanent stats.  Keep this on the
            // combat copy; reconciliation only commits the stat delta.
            if (trinketAvenger.dealDamage)
                combatField.ForEachAlive([](MinionData& data) {
                    data.value().SetHealth(data.value().GetHealth() - 1);
                });
        if (trinketAvenger.summonBeetles > 0) {
            const Card beetleCard = Cards::FindCardByID("BG28_603t");
            for (std::int32_t i = 0; i < trinketAvenger.summonBeetles; ++i) {
                if (combatField.IsFull()) {
                    owner.ApplySummonOverflowTrinkets();
                    break;
                }
                Minion beetle{beetleCard};
                owner.ApplyFreshMinionModifiers(beetle);
                beetle.getPlayerCallback = [&owner]() -> Player& {
                    return owner;
                };
                if (owner.getNextCardIndexCallback)
                    beetle.SetIndex(owner.getNextCardIndexCallback());
                combatField.Add(beetle);
                Minion& summoned = combatField[combatField.GetCount() - 1];
                summoned.SetTaunt(true);
                owner.ApplyTamuzoCombatSummon(summoned);
                combatField.ForEachAlive([&summoned](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::SUMMON, summoned);
                });
                owner.ApplySummonTrinkets(summoned);
            }
        }
            owner.ResolveBoomController(combatField);
        // Cloud Serpent Horn resolves after the complete death boundary:
        // choose the current right-most surviving friendly minion as the
        // source, then a different friendly Dragon as the recipient.  The
        // printed "Give the Attack" copies the source's current Attack; it
        // does not drain the source.  Resolve once per triggered Horn copy.
        // Both entities are combat copies, so commit only the granted Attack
        // to the matching recruit entity; no hidden target or future board
        // state is consulted.
        for (std::int32_t transfer = 0;
             transfer < trinketAvenger.transferRightmostAttackToDragon;
             ++transfer) {
            Minion* source = nullptr;
            std::vector<Minion*> dragons;
            combatField.ForEachAlive([&](MinionData& data) {
                auto& minion = data.value();
                source = &minion;
                if (minion.HasRace(Race::DRAGON)) dragons.push_back(&minion);
            });
            if (source != nullptr && source->GetAttack() > 0) {
                dragons.erase(
                    std::remove(dragons.begin(), dragons.end(), source),
                    dragons.end());
                if (!dragons.empty()) {
                    Minion* target =
                        dragons[Random::get<std::size_t>(0, dragons.size() - 1)];
                    const int attack = source->GetAttack();
                    target->SetAttack(target->GetAttack() + attack);
                    const auto targetIndex = target->GetIndex();
                    owner.recruitField.ForEachAlive(
                        [targetIndex, attack](MinionData& data) {
                            auto& recruit = data.value();
                            if (recruit.GetIndex() == targetIndex)
                                recruit.SetAttack(recruit.GetAttack() + attack);
                        });
                }
            }
        }
        // Battle Horn fires one friendly Battlecry for each completed Avenge
        // threshold.  Resolve the selected source directly through its
        // canonical POWER task so the normal Battlecry payload runs without
        // recursively counting another Trinket trigger.
        for (std::int32_t trigger = 0;
             trigger < trinketAvenger.triggerFriendlyBattlecry; ++trigger) {
            std::vector<Minion*> candidates;
            combatField.ForEachAlive([&candidates](MinionData& data) {
                if (data.value().HasBattlecry())
                    candidates.push_back(&data.value());
            });
            if (candidates.empty()) break;
            auto* selected = candidates[
                Random::get<std::size_t>(0, candidates.size() - 1)];
            selected->ActivateTask(PowerType::POWER, owner);
        }
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
    // Rapid Reanimation is a delayed observer: its target's Deathrattle may
    // have filled the freed slot. Re-check after the complete death batch,
    // and against the combat field so a later death in this combat can open
    // space for the exact snapshot.
    (void)m_player1.TryResolveRapidReanimationIfSpace(m_p1Field);
    (void)m_player2.TryResolveRapidReanimationIfSpace(m_p2Field);
    (void)m_player1.TryResolveSoulFermenterIfSpace(m_p1Field);
    (void)m_player2.TryResolveSoulFermenterIfSpace(m_p2Field);
    // S'Thara is a post-death empty-board trigger.  Resolve only after the
    // complete death/deathrattle boundary so Reborn and generated summons
    // can legitimately prevent the "last minion" condition.
    (void)m_player1.ResolveLastFriendlyDeathDemon();
    (void)m_player2.ResolveLastFriendlyDeathDemon();
    TryFireQueuedLockAndLoad();
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
