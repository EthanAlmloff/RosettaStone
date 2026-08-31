// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/GameEnums.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
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
    ,CHOOSE_ONE = 6
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
struct Season14ChooseOneState { bool pending = false; std::uint64_t sourceEntityID = 0; std::uint32_t targetMask = 0; std::int32_t sourceCardDbfID = 0; };

//! State for a modal originating from a Tavern spell.  Unlike minion
//! Choose-One state, the source is consumed from hand before the decision is
//! presented, so the source DBF and target slot are retained explicitly.
enum class Season14SpellModalKind : std::uint8_t
{
    NONE,
    TARGET_STATS,
    ALL_MINION_STATS,
    TARGET_OR_ALL_STATS
};
struct Season14SpellModalState {
    Season14SpellModalKind kind = Season14SpellModalKind::NONE;
    std::int32_t sourceCardDbfID = 0;
    std::int32_t targetIndex = -1;
    std::uint64_t targetEntityID = 0;
    std::int32_t firstAttack = 0, firstHealth = 0;
    std::int32_t secondAttack = 0, secondHealth = 0;
    bool secondBranchDelayed = false;
    std::string offeringFilter;
};
struct Season14PendingCombatBuff
{
    std::int32_t sourceCardDbfID = 0;
    std::uint64_t targetEntityID = 0;
    std::int32_t attack = 0;
    std::int32_t health = 0;
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
    //! Identity of the effect that created the public offering. These fields
    //! make a pending modal replayable and prevent callers from treating an
    //! offering as an anonymous global random result.
    std::uint64_t pendingSourceEntityID = 0;
    std::int32_t pendingSourceCardDbfID = 0;
    Season14ChooseOneState chooseOne;
    Season14SpellModalState spellModal;
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
    Season14HeroPowerBatch5State heroPowerBatch5;

    //! Batch 3 currently contains stateless combat/activation families.  It
    //! is kept as an explicit state member for schema clarity and future
    //! counters, while all unsupported families remain fail-closed.
    std::uint8_t heroPowerBatch3State = 0;

    //! Hooks for effects whose entity behavior is implemented elsewhere.
    bool lockboxActive = false;
    std::int32_t lockboxAdvance = 0;
    bool fishbaitActive = false;
    bool pendingHandLock = false;
    //! Deferred/economy state used by simple Tavern spells.  These counters
    //! are player-owned so refreshing or replacing a Tavern cannot lose a
    //! spell's intended duration.
    std::int32_t nextTurnGold = 0;
    std::int32_t pendingCombatRewardDbfID = 0;
    std::vector<Season14PendingCombatBuff> pendingCombatBuffs;
    std::vector<std::int32_t> pendingCombatStartEffects;
    //! Number of BG28_884 casts waiting for the same upcoming combat.
    std::int32_t pendingCombatRewardCount = 0;
    std::int32_t minionsPlayedThisTurn = 0;
    std::int32_t battlecriesTriggered = 0;
    std::int32_t deathrattlesTriggered = 0;
    std::int32_t maxGoldDelta = 0;
    std::int32_t freeRefreshes = 0;
    std::int32_t persistentShopAttack = 0;
    std::int32_t persistentShopHealth = 0;
    //! Cumulative improvement applied to newly created Tasty Lobsters.  This
    //! is player-owned game state, so it survives combat, deaths, Tavern
    //! refreshes, and recruit-phase transitions.
    std::int32_t futureLobsterAttack = 0;
    std::int32_t futureLobsterHealth = 0;
    std::int32_t futureBallerAttack = 0;
    std::int32_t futureBallerHealth = 0;
    std::int32_t trinketExtraShopSlots = 0;
    std::int32_t trinketHigherTierRefreshes = 0;
    std::int32_t trinketMaxGoldDelta = 0;
    std::int32_t trinketImmediateGold = 0;
    //! Number of end-of-recruit effects that increase the next turn's gold cap.
    std::int32_t trinketEndTurnMaxGold = 0;
    std::vector<Season14RaceShopStats> persistentShopRaceStats;
    std::vector<Season14RaceShopStats> persistentRaceStats;
    std::int32_t refreshRandomShopAttack = 0;
    std::int32_t refreshRandomShopHealth = 0;
    std::int32_t fodderRefreshes = 0;
    std::int32_t goldenMinionsPlayed = 0;
    std::int32_t unboundElementals = 0;
    std::vector<std::string> combatAvengeCards;
    std::int32_t foddersPerRefresh = 1;
    //! Cumulative health added by future Tavern spells this game.
    std::int32_t tavernSpellHealthBonus = 0;
    //! Cumulative attack added by future Tavern spells this game.
    std::int32_t tavernSpellAttackBonus = 0;
    std::int32_t deferredMinionAttack = 0;
    std::int32_t deferredMinionHealth = 0;
    std::uint8_t deferredMinionStatTurns = 0;
    std::int32_t persistentBeetleAttack = 0;
    std::int32_t persistentBeetleHealth = 0;
    std::int32_t nextTavernSpellDiscount = 0;
    std::int32_t persistentTavernTierAttack = 0;
    std::int32_t persistentTavernTierHealth = 0;
    std::int32_t persistentTavernTierMax = 0;
    struct GrowingSummonBonus { std::int32_t entityIndex; std::int32_t nextAttack; std::int32_t increment; };
    std::vector<GrowingSummonBonus> growingSummonBonuses;
    //! Additive attack/health applied by each subsequently played Blood Gem.
    std::int32_t bloodGemAttackBonus = 0;
    std::int32_t bloodGemHealthBonus = 0;
    std::vector<Season14RaceShopStats> bloodGemRaceBonuses;
    std::int32_t goldSpentThisTurn = 0;
    std::array<std::uint64_t, static_cast<std::size_t>(
                                  Season14Event::COUNT)>
        eventCounts{};

    //! Replaces the current public modal offering and enters its decision.
    void BeginDecision(Season14Decision decision,
                       std::vector<Season14Offering> offerings);
    void BeginOfferingDecision(Season14Decision decision,
                               std::uint64_t sourceEntityID,
                               std::int32_t sourceCardDbfID,
                               std::vector<Season14Offering> offerings);

    //! Selects one public offering and clears the pending decision.
    //! \return false when no matching pending offering exists.
    bool SelectDecision(std::size_t offeringIndex);
    void BeginChooseOne(std::uint64_t sourceEntityID, std::uint32_t targetMask, std::int32_t sourceCardDbfID, std::vector<Season14Offering> offerings);
    void BeginSpellTargetChoice(std::int32_t sourceCardDbfID, std::int32_t targetIndex,
                                std::uint64_t targetEntityID,
                                std::int32_t firstAttack, std::int32_t firstHealth,
                                std::int32_t secondAttack, std::int32_t secondHealth,
                                std::string offeringFilter = {},
                                Season14SpellModalKind kind = Season14SpellModalKind::TARGET_STATS);
    void BeginSpellAllMinionChoice(Season14SpellModalKind kind,
                                   std::int32_t sourceCardDbfID,
                                   std::int32_t firstAttack,
                                   std::int32_t firstHealth,
                                   std::int32_t secondAttack,
                                   std::int32_t secondHealth,
                                   bool secondBranchDelayed = false);
    bool SelectSpellTargetChoice(std::size_t offeringIndex,
                                 std::int32_t& attack, std::int32_t& health);

    //! Configures the hero power without claiming its behavior is supported.
    void SetHeroPower(std::int32_t dbfID, std::int32_t cost, bool available);

    //! Applies deterministic hero-power hooks at the start of recruit.
    //! The result contains effects paid immediately by Player/Game.
    Season14HeroPowerBatch2Result BeginRecruitTurn();

    //! Advances target-free Batch-4 lifecycle state at recruit start.
    void BeginRecruitTurnBatch4();
    void BeginRecruitTurnBatch5();

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
    bool ResolveHeroPowerBatch5Activation(
        Season14HeroPowerBatch5Result& result,
        std::int32_t tier = 1) const noexcept;
    bool ApplyHeroPowerBatch5Activation(
        Season14HeroPowerBatch5Result& result,
        std::int32_t tier = 1) noexcept;

    //! Arms the next Tavern fill with minions from one tier above the player.
    //! The count is consumed by MinionPool when that fill occurs.
    void ArmHigherTierRefresh(std::int32_t count);

    //! Returns and clears the one-shot higher-tier refresh allowance.
    std::int32_t TakeHigherTierRefresh();

    //! Returns whether this hero automatically freezes the remaining Tavern.
    bool ShouldFreezeRemainingTavern() const;

    //! Returns the effective cost of a Tavern spell before resolution.
    std::int32_t TavernSpellCost(std::int32_t baseCost) const;
    std::int32_t ConsumeTavernSpellDiscount() noexcept;

    //! Returns the resolved stats of one Blood Gem, including persistent
    //! hero/effect scaling.
    std::pair<std::int32_t, std::int32_t> BloodGemStats() const noexcept;
    std::pair<std::int32_t, std::int32_t> BloodGemStatsFor(Race race) const noexcept;

    //! Returns only the cumulative race-scoped Blood Gem bonus.  Callers
    //! holding a multi-type/ALL minion may query each matching type.
    std::pair<std::int32_t, std::int32_t> BloodGemRaceStatsFor(
        Race race) const noexcept;

    //! Adds persistent Blood Gem scaling for future generated/played gems.
    void AddBloodGemBonus(std::int32_t attack, std::int32_t health) noexcept;
    void AddTavernSpellHealthBonus(std::int32_t health) noexcept;
    void AddTavernSpellAttackBonus(std::int32_t attack) noexcept;
    std::int32_t TakeGrowingSummonAttack(std::int32_t entityIndex,
                                          std::int32_t initialAttack,
                                          std::int32_t increment);
    void AddBloodGemRaceBonus(Race race, std::int32_t attack,
                              std::int32_t health);

    //! Records a successful Tavern refresh so one-shot effects are consumed.
    void OnRefreshTavern(bool refreshSucceeded);

    //! Records a successfully resolved Tavern spell.
    void OnTavernSpellResolved(bool spellResolved);

    //! Adds deferred gold paid at the next recruit start.
    void AddNextTurnGold(std::int32_t amount) noexcept;
    void ArmNextCombatReward(std::int32_t sourceCardDbfID) noexcept;
    void ResolveNextCombatReward(BattleResult result, bool playerOne) noexcept;
    void ArmNextCombatBuff(std::int32_t sourceCardDbfID,
                           std::uint64_t targetEntityID,
                           std::int32_t attack,
                           std::int32_t health) noexcept;
    bool ResolveNextCombatBuff(BattleResult result, bool playerOne,
                               std::vector<Season14PendingCombatBuff>& resolved) noexcept;
    void ArmCombatStartLeftmostAttackDouble(std::int32_t sourceCardDbfID) noexcept;
    std::size_t TakeCombatStartLeftmostAttackDoubles() noexcept;
    void ArmCombatStartNearestStats(std::int32_t sourceCardDbfID) noexcept;
    std::size_t TakeCombatStartNearestStats() noexcept;
    void RecordMinionPlay(bool battlecry) noexcept
    {
        ++minionsPlayedThisTurn;
        if (battlecry) ++battlecriesTriggered;
    }
    void RecordBattlecry() noexcept { ++battlecriesTriggered; }
    int RecordGoldSpent(std::int32_t amount) noexcept;

    //! Returns and clears deferred next-turn gold.
    std::int32_t TakeNextTurnGold() noexcept;

    //! Returns and clears gold granted when a Trinket was selected.
    std::int32_t TakeImmediateGold() noexcept;

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

    //! Improves every future Tasty Lobster created for this player.
    void ImproveFutureLobsters(std::int32_t attack,
                               std::int32_t health) noexcept;

    //! Returns the cumulative future-Lobster bonus.
    std::pair<std::int32_t, std::int32_t> FutureLobsterStats() const noexcept;
    void ImproveFutureBallers(std::int32_t attack, std::int32_t health);
    std::pair<std::int32_t, std::int32_t> FutureBallerStats() const noexcept;

    //! Adds a persistent Tavern stat bonus scoped to a concrete tribe.
    void AddPersistentShopRaceStats(Race race, std::int32_t attack,
                                    std::int32_t health);
    void AddPersistentRaceStats(Race race, std::int32_t attack,
                                std::int32_t health);

    //! Arms a persistent trigger that buffs one random fresh Tavern minion
    //! after each successful refresh.
    void ArmRefreshRandomShopStats(std::int32_t attack,
                                   std::int32_t health) noexcept;

    //! Returns the armed random-refresh bonus, or zeroes when none is armed.
    std::pair<std::int32_t, std::int32_t>
    RefreshRandomShopStats() noexcept;
    void ArmFodderRefreshes(std::int32_t count, std::int32_t amount = 1) noexcept { if (count > 0) { fodderRefreshes += count; foddersPerRefresh = std::max(foddersPerRefresh, amount); } }
    std::int32_t ConsumeFodderRefresh() noexcept { if (fodderRefreshes <= 0) return 0; --fodderRefreshes; const auto result = foddersPerRefresh; if (fodderRefreshes == 0) foddersPerRefresh = 1; return result; }
    void TrackCombatAvengeCard(std::string id) { combatAvengeCards.push_back(std::move(id)); }
    std::vector<std::string> TakeCombatAvengeCards() { auto result = std::move(combatAvengeCards); combatAvengeCards.clear(); return result; }

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
