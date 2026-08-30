// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch4.hpp>
#include <Rosetta/Common/Enums/CardEnums.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
//! Public modal decisions introduced by modern Battlegrounds content.
enum class Season14Decision : std::int32_t
{
    NONE = 0,
    CHOICE = 2,
    DISCOVER = 3,
    TRINKET_SELECTION = 4,
    DARK_GIFT_SELECTION = 5
};

//! Events at which modern effects may be activated.
enum class Season14Event : std::uint8_t
{
    RECRUIT_START,
    RECRUIT_END,
    COMBAT_START,
    COMBAT_END,
    SPELL_CAST,
    COUNT
};

//! A public offering. Hidden simulator state must never be put in this list.
struct Season14Offering
{
    std::int32_t dbfID = 0;
    std::uint64_t entityID = 0;
};

//! Persistent Trinket or Dark Gift state owned by one player.
struct Season14PersistentEffect
{
    std::int32_t dbfID = 0;
    std::uint8_t remainingUses = 0;
    bool active = true;
};

//! A persistent Tavern stat bonus scoped to one or more concrete tribes.
//! The vector is intentionally player-owned so the effect survives Tavern
//! refreshes without exposing hidden pool state to the bridge.
struct Season14RaceShopStats
{
    Race race = Race::INVALID;
    std::int32_t attack = 0;
    std::int32_t health = 0;
};

inline constexpr std::size_t SEASON14_TRINKET_SLOTS = 2;
inline constexpr std::size_t SEASON14_DARK_GIFT_SLOTS = 16;
//! Small, simulator-independent state machine for modern modal mechanics.
class Season14State
{
 public:
    Season14Decision pendingDecision = Season14Decision::NONE;
    std::vector<Season14Offering> choiceOfferings;
    std::vector<Season14Offering> pendingOfferings;
    std::vector<Season14PersistentEffect> trinkets;
    std::vector<Season14PersistentEffect> darkGifts;

    std::int32_t heroPowerDbfID = 0;
    std::int32_t heroPowerCost = 0;
    bool heroPowerAvailable = false;
    bool heroPowerUsed = false;

    //! State for the currently implemented modern hero-power families.
    //! Keeping this with the player-owned Season 14 state makes discounts and
    //! lifecycle counters available to both the event hooks and Player's
    //! action-cost checks.
    Season14HeroPowerBatch1State heroPowerBatch1;
    Season14HeroPowerBatch2State heroPowerBatch2;
    Season14HeroPowerBatch4State heroPowerBatch4;

    //! Batch 3 currently contains stateless combat/activation families.  It
    //! is kept as an explicit state member for schema clarity and future
    //! counters, while all unsupported families remain fail-closed.
    std::uint8_t heroPowerBatch3State = 0;

    //! Hooks for effects whose entity behavior is implemented elsewhere.
    bool lockboxActive = false;
    bool fishbaitActive = false;
    //! Deferred/economy state used by simple Tavern spells.  These counters
    //! are player-owned so refreshing or replacing a Tavern cannot lose a
    //! spell's intended duration.
    std::int32_t nextTurnGold = 0;
    std::int32_t maxGoldDelta = 0;
    std::int32_t freeRefreshes = 0;
    std::int32_t persistentShopAttack = 0;
    std::int32_t persistentShopHealth = 0;
    std::vector<Season14RaceShopStats> persistentShopRaceStats;
    std::int32_t refreshRandomShopAttack = 0;
    std::int32_t refreshRandomShopHealth = 0;
    std::array<std::uint64_t, static_cast<std::size_t>(
                                  Season14Event::COUNT)>
        eventCounts{};

    //! Replaces the current public modal offering and enters its decision.
    void BeginDecision(Season14Decision decision,
                       std::vector<Season14Offering> offerings);

    //! Selects one public offering and clears the pending decision.
    //! \return false when no matching pending offering exists.
    bool SelectDecision(std::size_t offeringIndex);

    //! Configures the hero power without claiming its behavior is supported.
    void SetHeroPower(std::int32_t dbfID, std::int32_t cost, bool available);

    //! Applies deterministic hero-power hooks at the start of recruit.
    //! The result contains effects paid immediately by Player/Game.
    Season14HeroPowerBatch2Result BeginRecruitTurn();

    //! Advances target-free Batch-4 lifecycle state at recruit start.
    void BeginRecruitTurnBatch4();

    //! Resets per-combat Avenge progress before combat starts.
    void BeginCombatBatch4();

    //! Applies a successful minion sale to deferred hero-power state.
    void OnSellMinion();

    //! Returns immediate bonus gold for a successfully purchased minion.
    std::int32_t OnBuyMinion(bool purchasedPirate) const;

    //! Returns the one-time attack bonus for the next minion purchase.
    std::int32_t OnBuyMinionBatch4();

    //! Applies a successfully played Elemental to hero-power state.
    Season14HeroPowerBatch2Result OnPlayElemental();

    //! Applies a successful minion-play lifecycle event.
    Season14HeroPowerBatch4Result OnPlayMinionBatch4();

    //! Applies a combat death to an Avenge hero power.
    Season14HeroPowerBatch4Result OnFriendlyMinionDiedBatch4();

    //! Applies a successful Tavern upgrade and returns its gold/cost effects.
    Season14HeroPowerBatch2Result OnUpgradeTavern();

    //! Returns the effective cost of buying a minion under passive auras.
    std::int32_t MinionPurchaseCost(std::int32_t baseCost) const;

    //! Returns the effective cost of refreshing the Tavern.
    std::int32_t RefreshCost(std::int32_t baseCost) const;

    //! Returns the effective cost of upgrading the Tavern.
    std::int32_t UpgradeCost(std::int32_t baseCost) const;

    //! Returns the number of Tavern offers after passive hero modifiers.
    std::size_t TavernOfferCount(std::size_t baseCount) const;

    //! Returns passive fixed modifiers from the target-free Batch-4 family.
    Season14HeroPowerBatch4PassiveModifiers
    HeroPowerBatch4PassiveModifiers() const noexcept;

    //! Resolves the target-free Batch-4 activation, if complete.
    bool ResolveHeroPowerBatch4Activation(
        Season14HeroPowerBatch4Result& result) const noexcept;

    //! Commits an already-validated target-free Batch-4 activation.
    bool ApplyHeroPowerBatch4Activation(
        Season14HeroPowerBatch4Result& result) noexcept;

    //! Arms the next Tavern fill with minions from one tier above the player.
    //! The count is consumed by MinionPool when that fill occurs.
    void ArmHigherTierRefresh(std::int32_t count);

    //! Returns and clears the one-shot higher-tier refresh allowance.
    std::int32_t TakeHigherTierRefresh();

    //! Returns whether this hero automatically freezes the remaining Tavern.
    bool ShouldFreezeRemainingTavern() const;

    //! Returns the effective cost of a Tavern spell before resolution.
    std::int32_t TavernSpellCost(std::int32_t baseCost) const;

    //! Records a successful Tavern refresh so one-shot effects are consumed.
    void OnRefreshTavern(bool refreshSucceeded);

    //! Records a successfully resolved Tavern spell.
    void OnTavernSpellResolved(bool spellResolved);

    //! Adds deferred gold paid at the next recruit start.
    void AddNextTurnGold(std::int32_t amount) noexcept;

    //! Returns and clears deferred next-turn gold.
    std::int32_t TakeNextTurnGold() noexcept;

    //! Returns the configured maximum gold cap.
    std::int32_t EffectiveMaxGold(std::int32_t baseCap) const noexcept;

    //! Increases the maximum gold cap for future turns.
    void IncreaseMaxGold(std::int32_t amount) noexcept;

    //! Adds free Tavern refresh allowances.
    void AddFreeRefreshes(std::int32_t amount) noexcept;

    //! Returns whether a free refresh can be consumed.
    bool HasFreeRefresh() const noexcept;

    //! Consumes one free refresh after a successful refresh.
    bool ConsumeFreeRefresh() noexcept;

    //! Adds a persistent stat bonus to newly generated Tavern minions.
    void AddPersistentShopStats(std::int32_t attack,
                                std::int32_t health) noexcept;

    //! Adds a persistent Tavern stat bonus scoped to a concrete tribe.
    void AddPersistentShopRaceStats(Race race, std::int32_t attack,
                                    std::int32_t health);

    //! Arms a persistent trigger that buffs one random fresh Tavern minion
    //! after each successful refresh.
    void ArmRefreshRandomShopStats(std::int32_t attack,
                                   std::int32_t health) noexcept;

    //! Returns the armed random-refresh bonus, or zeroes when none is armed.
    std::pair<std::int32_t, std::int32_t>
    RefreshRandomShopStats() const noexcept;

    //! Resolves a complete target-free Batch-3 activation for this hero.
    bool ResolveHeroPowerBatch3Activation(
        std::int32_t currentTier,
        Season14HeroPowerBatch3Activation& result) const noexcept;

    //! Returns a passive combat-kill bonus owned by this hero, if any.
    std::int32_t HeroPowerBatch3CombatKillAttackBonus() const noexcept;

    //! Returns whether the player can pay and use the power this turn.
    bool CanUseHeroPower(std::int32_t availableGold) const;

    //! Returns the current cost after deterministic hero-power discounts.
    std::int32_t EffectiveHeroPowerCost() const;

    //! Consumes the power for this turn.
    bool UseHeroPower();

    //! Returns whether another Trinket fits in the persistent slots.
    bool CanAddTrinket() const;

    //! Returns whether another Dark Gift fits in the persistent slots.
    bool CanAddDarkGift() const;

    //! Adds a persistent effect to a player-visible slot.
    void AddTrinket(Season14PersistentEffect effect);
    void AddDarkGift(Season14PersistentEffect effect);

    //! Decrements a finite effect, removing it when it reaches zero.
    bool ConsumeEffect(std::vector<Season14PersistentEffect>& effects,
                       std::size_t slot);

    //! Records an event for effect scheduling/diagnostics.
    void Emit(Season14Event event);

    //! Generic index validation used by bridge target masks.
    static bool IsValidBoardTarget(std::int32_t index,
                                   std::int32_t boardCount);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
