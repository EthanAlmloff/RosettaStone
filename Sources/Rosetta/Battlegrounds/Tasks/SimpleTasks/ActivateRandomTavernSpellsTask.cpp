#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
// Battlegrounds seeds the thread-local generator from Game::m_seed.  Using
// random_static here would make Puzzle Box depend on process-global entropy,
// breaking seeded replay and making the automatic spell choice diverge from
// every other Battlegrounds random effect.
using Random = effolkronium::random_thread_local;
TaskStatus ActivateRandomTavernSpellsTask::Run(Player& p) {
    if (m_amount <= 0) return TaskStatus::STOP;
    const bool generatedRewardSequence =
        p.season14.HasGeneratedRewardStartTurnRandomSpells() &&
        p.season14.HasPendingGeneratedRewardRandomSpells();
    const bool lavishCapeSequence =
        !generatedRewardSequence &&
        p.season14.HasPendingLavishCapeRandomSpells();
    const int casts = generatedRewardSequence
        ? p.season14.GeneratedRewardRandomSpellsRemaining()
        : lavishCapeSequence
            ? p.season14.LavishCapeRandomSpellsRemaining()
            : m_amount;
    if (casts <= 0) {
        if (generatedRewardSequence)
            p.season14.FinishGeneratedRewardRandomSpells();
        else if (lavishCapeSequence)
            p.season14.FinishLavishCapeRandomSpells();
        return TaskStatus::COMPLETE;
    }
    std::vector<const Card*> pool;
    for (const auto& c : Cards::GetAllCards()) {
        if (!c.isBattlegroundsPoolSpell || c.normalDbfID != 0 ||
            (c.GetCardType() != CardType::SPELL &&
             c.GetCardType() != CardType::BATTLEGROUND_SPELL))
            continue;
        const auto behavior = FindTavernSpellBehavior(c.id);
        if (behavior.effect == TavernSpellEffect::NONE) continue;
        if (!TavernSpellRequiresTarget(behavior.effect)) {
            pool.push_back(&c);
            continue;
        }
        if (TavernSpellRequiresTarget(behavior.effect)) {
            const bool shopTarget = TavernSpellTargetsShop(behavior.effect);
            bool legalTarget = false;
            if (shopTarget) {
                p.tavern.fieldZone.ForEach([&](const MinionData& data) {
                    const auto& target = data.value();
                    legalTarget = legalTarget ||
                        (!target.IsDestroyed() && !target.GetCardID().empty() &&
                         ((behavior.effect != TavernSpellEffect::TARGET_SHOP_COPY &&
                           behavior.effect != TavernSpellEffect::TARGET_SHOP_COPY_TIER) ||
                          p.hand.GetCount() + behavior.value <= MAX_HAND_SIZE) &&
                         (behavior.effect != TavernSpellEffect::TARGET_SHOP_MOVE_NON_GOLDEN ||
                          (!p.hand.IsFull() && !target.IsGolden())) &&
                         (behavior.effect != TavernSpellEffect::TARGET_SHOP_COPY_TIER ||
                          target.GetTier() <= 3) &&
                         TavernSpellTargetIsLegal(behavior.effect,
                                                  target.GetTier(), target.IsGolden()));
                });
            } else {
                p.GetField().ForEachAlive([&](const MinionData&) {
                    legalTarget = true;
                });
                // `race` is not uniformly a target restriction.  Some
                // effects use it only for a conditional bonus after a legal
                // target is selected (for example Shifting Tide's Naga
                // repeat and Deepwater Clan's Murloc splash).  Restrict the
                // generated pool only where the executor actually rejects a
                // different target race.
                const bool targetMustMatchRace =
                    behavior.effect == TavernSpellEffect::TARGET_RANDOM_RACE_KEYWORD ||
                    behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND ||
                    behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_GIVE_PERSISTENT_ATTACK ||
                    (behavior.effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS &&
                     behavior.race != Race::INVALID);
                if (targetMustMatchRace) {
                    legalTarget = false;
                    p.GetField().ForEachAlive([&](const MinionData& data) {
                        legalTarget = legalTarget ||
                            data.value().HasRace(behavior.race);
                    });
                }
                if (behavior.effect == TavernSpellEffect::TARGET_SHARED_RACE_STATS ||
                    behavior.effect == TavernSpellEffect::TARGET_RACE_SHOP_STATS_PERSISTENT ||
                    behavior.effect == TavernSpellEffect::DISCOVER_DIFFERENT_RACE ||
                    behavior.effect == TavernSpellEffect::REFRESH_RACE) {
                    legalTarget = false;
                    p.GetField().ForEachAlive([&](const MinionData& data) {
                        legalTarget = legalTarget ||
                            data.value().GetRace() != Race::INVALID;
                    });
                }
                if (behavior.effect == TavernSpellEffect::TARGET_TRIGGER_DEATHRATTLE) {
                    legalTarget = false;
                    p.GetField().ForEachAlive([&](const MinionData& data) {
                        legalTarget = legalTarget || data.value().HasDeathrattle();
                    });
                }
                if (behavior.effect == TavernSpellEffect::TARGET_GOLDEN ||
                    behavior.effect == TavernSpellEffect::TARGET_GOLDEN_TEMPORARY) {
                    legalTarget = false;
                    p.GetField().ForEachAlive([&](const MinionData& data) {
                        const auto& target = data.value();
                        legalTarget = legalTarget ||
                            TavernSpellTargetIsLegal(behavior.effect,
                                                     target.GetTier(), target.IsGolden()) &&
                            target.CanMakeGolden();
                    });
                }
                if (behavior.effect == TavernSpellEffect::TARGET_RANDOM_RACE_KEYWORD) {
                    legalTarget = false;
                    p.GetField().ForEachAlive([&](const MinionData& data) {
                        const auto& target = data.value();
                        const bool hasOpenKeyword =
                            !target.HasDivineShield() || !target.HasReborn() ||
                            !target.HasWindfury() || !target.HasVenomous() ||
                            !target.HasTaunt() || !target.HasStealth();
                        legalTarget = legalTarget ||
                            hasOpenKeyword &&
                            (behavior.race == Race::INVALID ||
                             target.HasRace(behavior.race));
                    });
                }
                if (behavior.effect == TavernSpellEffect::TARGET_DOUBLE_STATS_HAND_LOCK) {
                    legalTarget = legalTarget && !p.hand.IsFull();
                }
                if (behavior.effect == TavernSpellEffect::DESTROY_UNDEAD_RANDOM_TO_HAND) {
                    // Free casts do not have a spell card in hand to consume;
                    // reserve every printed random reward slot and fail closed
                    // when the typed race pool is unavailable.
                    legalTarget = legalTarget && behavior.randomCount > 0 &&
                        p.hand.GetCount() + behavior.randomCount <= MAX_HAND_SIZE;
                }
                if (behavior.effect == TavernSpellEffect::TARGET_CONSUME_SHOP_STATS) {
                    std::size_t available = 0;
                    p.tavern.fieldZone.ForEach([&](const MinionData& data) {
                        if (!data.value().IsDestroyed() &&
                            data.value().GetPoolIndex() >= 0)
                            ++available;
                    });
                    legalTarget = legalTarget &&
                        available >= static_cast<std::size_t>(
                            std::max(0, behavior.randomCount));
                }
            }
            if (!legalTarget) continue;
        }
        pool.push_back(&c);
    }
    if (pool.empty()) {
        if (generatedRewardSequence)
            p.season14.FinishGeneratedRewardRandomSpells();
        else if (lavishCapeSequence)
            p.season14.FinishLavishCapeRandomSpells();
        return TaskStatus::STOP;
    }
    for (int i = 0; i < casts; ++i) {
        const auto& card = *pool[Random::get<std::size_t>(0, pool.size() - 1)];
        const auto& id = card.id;
        const auto behavior = FindTavernSpellBehavior(card.id);
        // Target-required effects pause on a public modal.  The selected
        // spell is retained in pendingTaughtSpell, so choosing a target
        // resumes this exact outcome instead of rolling a second spell.
        if (generatedRewardSequence)
            p.season14.SetGeneratedRewardRandomSpellsRemaining(casts - i - 1);
        else if (lavishCapeSequence)
            p.season14.SetLavishCapeRandomSpellsRemaining(casts - i - 1);
        if (TavernSpellRequiresTarget(behavior.effect))
            p.season14.pendingTaughtSpell = {true, 0,
                                              static_cast<std::int32_t>(card.dbfID)};
        if (!p.CastTavernSpellFree(id)) {
            if (generatedRewardSequence)
                p.season14.FinishGeneratedRewardRandomSpells();
            else if (lavishCapeSequence)
                p.season14.FinishLavishCapeRandomSpells();
            return TaskStatus::STOP;
        }
        if (p.season14.pendingDecision != Season14Decision::NONE)
            return TaskStatus::COMPLETE;
    }
    if (generatedRewardSequence)
        p.season14.FinishGeneratedRewardRandomSpells();
    else if (lavishCapeSequence)
        p.season14.FinishLavishCapeRandomSpells();
    return TaskStatus::COMPLETE;
}
TaskStatus ActivateRandomTavernSpellsTask::Run(Player& p, Minion& s) { if(m_amount<=0||s.IsDestroyed()) return TaskStatus::STOP; std::vector<std::string> pool; for(const auto& c:Cards::GetAllCards()) if(c.isBattlegroundsPoolSpell&&c.normalDbfID==0&&FindTavernSpellBehavior(c.id).effect!=TavernSpellEffect::NONE) pool.emplace_back(c.id); if(pool.empty()) return TaskStatus::STOP; for(int i=0;i<m_amount;++i){ const auto& id=pool[Random::get<std::size_t>(0,pool.size()-1)]; p.CastTavernSpellFree(id,1,s.GetZonePosition()); } return TaskStatus::COMPLETE; }
TaskStatus ActivateRandomTavernSpellsTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
