// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP

#include <Rosetta/Battlegrounds/Models/Hero.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Models/Tavern.hpp>
#include <Rosetta/Battlegrounds/Tasks/TaskStack.hpp>
#include <Rosetta/Battlegrounds/Zones/FieldZone.hpp>
#include <Rosetta/Battlegrounds/Zones/HandZone.hpp>

#include <array>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
class Battle;

//!
//! \brief Player class.
//!
//! This class stores various information that used in Battlegrounds.
//!
class Player
{
 public:
    //! Returns the field according the status.
    //! \return The field according the status.
    FieldZone& GetField();

    //! Initializes a Hero instance.
    //! \param idx The index of hero choices.
    void SelectHero(std::size_t idx);

    //! Prepare a list of minions in Tavern for purchase.
    void PrepareTavern();

    //! Replaces last turn's temporary Spellcraft cards and emits this turn's
    //! cards from the currently owned Spellcraft minions.
    void RefreshSpellcraft();

    //! Applies passive modifiers that belong to a newly created minion
    //! instance.  This is intentionally callable by pool and summon paths so
    //! passive hero powers do not depend on the minion first appearing in the
    //! Tavern.
    void ApplyFreshMinionModifiers(Minion& minion);
    //! Applies fresh-instance passives plus persistent Tavern-offer auras.
    //! Shop-only auras never leak onto hand or board summons.
    void ApplyFreshTavernMinionModifiers(Minion& minion);
    //! Increments lifetime Start-of-Combat spell improvements on owned cards.
    void IncrementStartCombatSpellImprovements();
    //! Installs the supported generated quest-reward aura on a fresh or
    //! existing minion. Unsupported modal options remain fail-closed.
    bool ApplyGeneratedQuestReward(std::int32_t dbfID);
    //! Resolves player-owned generated rewards at recruit end / combat start.
    void ResolveGeneratedQuestRewardEndTurn();
    void ResolveGeneratedQuestRewardStartCombat(FieldZone& combatField);
    void ResolveGeneratedQuestRewardDeath(Minion& deadMinion);
    void ResolveGeneratedQuestRewardSnickerSnacks();
    void ResolveGeneratedQuestRewardStartTurn();
    //! Relics of the Deep grants one Spellcraft at each recruit start.
    void ResolveRelicsOfTheDeepStartTurn();
    void ResolveMechGyverDeath();
    //! Arms Fodder refreshes from Woodland Defiler end-of-turn triggers.
    void ResolveFodderDefilerEndTurn();
    void ResolveEnigmaticHeadstoneEndTurn();
    bool AddGeneratedDiscoverCopy(const Card& card);
    //! Applies Tamuzo's combat-only summon multiplier to a newly summoned unit.
    void ApplyTamuzoCombatSummon(Minion& summoned);
    //! Resolves combat-only Trinkets that listen to a newly summoned minion.
    //! The call is made by every authoritative summon path after insertion
    //! and ordinary SUMMON observers have seen the entity.
    void ApplySummonTrinkets(Minion& summoned);
    void RefreshSousChefHeroPowerUses();
    //! Applies Watfin only after Detective for Hire commits a correct guess.
    void ResolveWatfinGuess(bool correct, const Card& guessedMinion);
    //! Resolves Zippers only when a canonical helpful-card pool is available.
    bool ResolveZippersDeathrattle();
    //! Gives +2/+2 to one random friendly minion of every occupied Tavern
    //! Tier, using the simulator RNG stream.
    void ApplyNaturalBalance();
    void ApplyPersistentRaceStats(Race race, int attack, int health);
    //! Adds a cumulative race aura while excluding one triggering instance.
    //! This supports "other [race]" discover/event effects without losing
    //! the aura for minions created after the event.
    void ApplyPersistentRaceStatsExcept(Race race, int attack, int health,
                                        std::uint64_t excludedEntityID);
    //! Resolves discover-triggered player auras after a choice is committed.
    void ResolveDiscoverTriggers();
    void ApplySpellRaceBuff(Race race, int attack, int health, bool includeHand);
    void ApplySpellSpecialBuff(int mode, int attack, int health);
    //! Resolve a supported Tavern spell without charging gold. Used by
    //! combat-start card effects such as Runic Arcanist.
    bool CastTavernSpellFree(const std::string& cardID, int amount = 1,
                             int targetIdx = -1);
    void ApplyTavernRaceBuff(Race race, int attack, int health);
    void ArmNextBoughtStats(int sourceIndex, int multiplier);

    //! Purchases a minion from Tavern's field.
    //! \param idx The index of a list of minions in Tavern's field.
    void PurchaseMinion(std::size_t idx);
    //! Moves a Tavern minion to hand without purchase cost. Used by hero
    //! powers that acquire a shop entity directly; pool ownership remains
    //! with the moved instance.
    bool TakeTavernMinionToHand(std::size_t idx, int attack, int health);
    //! Each friendly Demon consumes one random Tavern minion for its stats.
    bool DevourRandomTavernForDemons(int multiplier);
    void UpdateSkyGolemsForDeathrattle();
    //! Summons an exact state copy of a friendly minion with a fresh entity
    //! identity. Used by Cloning Gallery; does not charge or consume hand.
    bool SummonExactMinionCopy(std::size_t idx);
    bool SummonCombatSnapshot(Minion snapshot);
    //! Adds a metadata-only (plain) copy of the left-most hand card.
    //! Dynamic buffs/enchantments are intentionally not copied.
    bool AddPlainCopyOfLeftmostHandCard();
    //! Adds a card-definition copy of a friendly board minion to hand.
    //! Dynamic instance state is intentionally not copied.
    bool AddMinionCopyToHand(const Minion& source);
    //! Begins Void Power's one-time Tier-5 Discover when its unlock fires.
    bool BeginVoidPowerDiscover();
    bool CanPurchaseTavernSlot(std::size_t idx) const;
    bool PurchaseTavernSlot(std::size_t idx);

    //! Plays a minion or spell card.
    //! \param handIdx The index of a list of cards in player's hand.
    //! \param fieldIdx The index of player's field to add.
    //! \param targetIdx The index of the target in player's field.
    void PlayCard(std::size_t handIdx, std::size_t fieldIdx,
                  int targetIdx = -1);

    //! Returns whether a supported no-target Tavern spell can be played.
    //! \param handIdx The index of the spell in the player's hand.
    bool CanPlaySpell(std::size_t handIdx) const;

    //! Returns whether a supported Tavern spell can be played on a friendly
    //! board target. A target of -1 denotes a no-target spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \param targetIdx The friendly board slot, or -1 for no target.
    bool CanPlaySpell(std::size_t handIdx, int targetIdx) const;

    //! Pays for and resolves a supported no-target Tavern spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \return false when the card, cost, or behavior is unsupported.
    bool PlaySpell(std::size_t handIdx);

    //! Pays for and resolves a supported targeted or no-target Tavern spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \param targetIdx The friendly board slot, or -1 for no target.
    //! \return false when the card, target, cost, or behavior is unsupported.
    bool PlaySpell(std::size_t handIdx, int targetIdx);

    //! Creates up to `count` Blood Gem spells in the player's hand.  The
    //! generated card keeps the canonical BG20_GEM identity so replay and
    //! legality use the same path as a naturally generated gem.
    int AddBloodGems(int count);
    //! Applies one canonical Blood Gem to an existing friendly minion.  This
    //! keeps generated effects (such as Rally) on the same aura/keyword
    //! path as a Blood Gem spell played from hand.
    void ApplyBloodGemTo(Minion& target);

    //! Creates up to `count` canonical Tavern Coin spells in hand.
    int AddTavernCoins(int count);
    //! Resolves Mister Clocksworth's two-copy golden threshold.  The
    //! consumed duplicate is replaced by one canonical Tavern Coin rather
    //! than a normal Triple Reward.
    bool ResolveDoubleTimeCopies();
    //! Resolves Sneed's starting Shredder Deathrattle in combat.
    bool ResolveSneedShredderDeathrattle(bool golden = false);
    //! Acquires a Trinket and applies any deterministic acquisition-time grant.
    //! This is the sole player-owned entry point for generated Trinket cards.
    bool AcquireTrinket(Season14PersistentEffect effect);
    int GrantTrinketStartTurnCards();

    //! Resolves a pending public Choice/Discover offering into the player's
    //! hand.  Only concrete minion and spell cards are accepted; unsupported
    //! modal effects remain pending and fail closed.
    bool ApplyChoice(std::size_t offeringIdx);
    //! Applies Conviction's selected improvement option and its random
    //! friendly-minion buff. The modal itself is replayable through Choice.
    bool ApplyConvictionHeroPower();
    //! Resolve a completed Dungar flightpath at recruit start.
    bool ResolveFlightpathCompletion();
    //! Sell one friendly minion and transfer its current stats to another.
    bool ApplyDevour(std::size_t sourceIdx, std::size_t targetIdx);
    //! Begin I Spy's public Discover from the next opponent's visible warband.
    bool BeginISpyDiscover();
    //! Adds the next opponent's public hero-linked Buddy at recruit start.
    int ResolveWardenBuddy();
    //! Begin Power of the Storm's two-option hero-power choice.
    bool BeginPowerOfStormChoice();
    //! Begin a seeded Discover offering of supported Tavern spells.
    bool BeginTavernSpellDiscover(int amount, std::uint64_t sourceEntityID,
                                  std::int32_t sourceCardDbfID);
    bool ApplyChooseOne(std::size_t offeringIdx, std::size_t targetIdx);
    //! Resolves a pending Tavern-spell modal without re-paying the spell.
    bool ApplySpellChoice(std::size_t offeringIdx);
    //! Resolves persistent Trinket effects after any successful Tavern spell,
    //! including modal/Choose-One completion paths.
    void ApplyTavernSpellTrinkets();
    void ApplyAfterPlayCardTrinkets(Race playedRace = Race::INVALID);
    void ApplyAfterRebornTrinkets();
    void ApplyStartCombatTrinkets();
    void ResolveStartTurnTrinkets();
    bool ShouldDuplicateDragonBattlecry() const noexcept;
    void ApplyFirstMinionDivineShield(Minion& minion);
    void ApplyDeferredTavernSpellStats();

    //! Applies a fully resolved target-free Season 14 hero-power activation.
    //! Random recipient selection uses RosettaStone's shared RNG stream.
    bool ApplySeason14HeroPowerBatch3Activation(
        const Season14HeroPowerBatch3Activation& activation);
    //! Applies a Batch-3 activation to the entity IDs selected during
    //! resolution.  The bridge uses this replay-safe path for effects whose
    //! random recipients must remain stable across duplicated resolution.
    bool ApplySeason14HeroPowerBatch3ResolvedTargets(
        const Season14HeroPowerBatch3Activation& activation,
        const std::array<std::uint64_t, 32>& entityIDs,
        std::uint8_t entityCount);
    bool ApplyArcaneAlteration(std::size_t slot, std::uint64_t entityID,
                               std::int32_t replacementDbfID);
    bool ApplySwapShopMinion(std::size_t boardSlot, std::uint64_t boardEntityID,
                             std::size_t tavernSlot, std::uint64_t tavernEntityID);

    //! Applies a manual Activate action from a recruit-board minion.
    bool ActivateMinion(std::size_t boardIdx, int targetIdx = -1);

    //! Sells a minion to Tavern.
    //! \param idx The index of a list of minions in player's field.
    void SellMinion(std::size_t idx);

    //! Upgrades your Tavern to the next tier.
    void UpgradeTavern();

    //! Refreshes a list of minions in Tavern's field.
    //! \p freeRefresh is used by a hero power whose activation already paid
    //! for the refresh (for example Temporal Tavern).
    void RefreshTavern(bool freeRefresh = false);
    void RecordGoldSpent(std::int32_t amount);

    //! Freezes a list of minions in Tavern's field.
    void FreezeTavern();

    //! Rearranges a minion to another position on player's field.
    //! \param curIdx The current index of minion.
    //! \param newIdx The new index of minion.
    void RearrangeMinion(std::size_t curIdx, std::size_t newIdx);

    //! Completes recruit phase.
    void CompleteRecruit();
    void ResolveRecruitEndDeaths();
    void ResolveDarkGiftEndTurnTriggers();
    //! Resolve Sulfuras' end-of-recruit trigger on the left/right edges.
    void ResolveSulfurasEndTurn();
    void ResolveCthunEndTurn();
    void AdvanceCthunUpgrade() noexcept;
    void ResolveTierMinionStartCombat();
    void TryDeliverChampionReward();
    void TryDeliverHeroicInspirationReward();
    void MaybeBeginExpeditionDiscovery();
    bool ArmLockAndLoad(std::size_t tavernIndex);
    void ResolveLockAndLoad();
    void BeginExpeditionDiscoveryForTier(int tier);
    void DeliverExpeditionReward();
    //! Advances all per-minion Dark Gift counters after a matching event.
    void AdvanceDarkGiftCounters(int kind);

    //! Processes the tasks related to defeat.
    void ProcessDefeat();

    //! Dispatches a successfully resolved hero-damage event to the active
    //! minions in the same zone as the damage.  Armor-only damage never
    //! reaches this lifecycle.
    void DispatchHeroDamage(const HeroDamageEvent& event);
    //! Dispatches damage dealt by a hero power to Buddy listeners.
    void DispatchHeroPowerDamage(int damage);
    //! Dispatches a positive persistent attack gain to friendly listeners.
    //! The event is emitted by Minion's explicit persistent-stat APIs only.
    void DispatchMinionAttackGain(Minion& target, int amount);
    void CheckAzsharaAmbition();
    //! Commits a supported damaging hero-power activation and dispatches the
    //! actual damage exactly once. Generic card damage must not use this.
    bool ResolveDamagingHeroPower(int actualDamage);

    PlayState playState = PlayState::INVALID;
    std::size_t idx = 0;
    std::size_t rank = 1;

    Hero hero;

    int remainCoin = 0;
    int totalCoin = 0;
    int armor = 0;
    int currentTier = 0;
    int coinToUpgradeTavern = 0;

    Tavern tavern;
    HandZone hand;
    FieldZone recruitField;
    FieldZone battleField;

    TaskStack taskStack;
    Season14State season14;
    //! Lifetime successful magnetizations, visible to supported scaling
    //! combat effects (never incremented for rejected/stale actions).
    int magnetizationsThisGame = 0;
    //! Prevents Beatboxer mirror applications from recursively mirroring.
    bool magnetizationMirrorInProgress = false;
    int malchezaarRefreshesRemaining = 0;
    int gemDays = 0;
    int combinedChooseOneUses = 0;
    //! Lifetime friendly Eternal Knight deaths used by its wherever-this-is
    //! aura; incremented only by authoritative death processing.
    int eternalKnightsDiedThisGame = 0;

    std::function<void(Player&)> selectHeroCallback;
    std::function<void(Player&)> prepareTavernMinionsCallback;
    std::function<void(Player&, std::size_t)> purchaseMinionCallback;
    std::function<bool(Player&, int)> addRandomTavernMinionCallback;
    std::function<bool(Player&, int)> addRandomMinionToHandCallback;
    std::function<int()> getNextCardIndexCallback;
    std::function<void(int)> returnMinionCallback;
    std::function<void(Player&)> clearTavernMinionsCallback;
    std::function<void(Player&)> upgradeTavernCallback;
    std::function<void()> completeRecruitCallback;
    std::function<Player&(Player&)> getOpponentPlayerCallback;
    std::function<Battle&()> getBattleCallback;
    std::function<void(Player&)> processDefeatCallback;

    std::array<int, 4> heroChoices{ 0, 0, 0, 0 };

    std::size_t playerIdxNextFight = std::numeric_limits<std::size_t>::max();
    std::size_t playerIdxFoughtLastTurn =
        std::numeric_limits<std::size_t>::max();

    bool isInCombat = false;
    bool isFoughtGhostLastTurn = false;
    bool freezeTavern = false;
    bool dispatchingMinionAttackGain = false;
    std::vector<std::pair<int, int>> nextBoughtStatsArms;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP
