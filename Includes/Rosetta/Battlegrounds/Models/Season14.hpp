// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/CardSets/FishbaitBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch6.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch10.hpp>
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/GameEnums.hpp>
#include <Rosetta/Common/Constants.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <optional>

namespace RosettaStone::Battlegrounds
{
class Player;
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
    std::int32_t darkGiftDbfID = 0;
};

// Synthetic, player-visible option IDs for Conviction's improvement modal.
// They deliberately do not resolve to cards: the modal is a typed choice,
// not a card Discover, and ApplyChoice handles these IDs only with source
// BG21_HERO_000p.
inline constexpr std::int32_t CONVICTION_IMPROVE_ATTACK = -739420;
inline constexpr std::int32_t CONVICTION_IMPROVE_HEALTH = -739421;
inline constexpr std::int32_t CONVICTION_IMPROVE_TARGETS = -739422;

//! Persistent Trinket or Dark Gift state owned by one player.
struct Season14PersistentEffect
{
    std::int32_t dbfID = 0;
    std::uint8_t remainingUses = 0;
    bool active = true;
    //! Trigger progress owned by this effect (for cadence-based Trinkets).
    //! It must not be shared between distinct Trinket instances.
    std::int32_t triggerProgress = 0;
    //! Per-instance improvement level for progressive Trinket payloads.
    std::int32_t statScale = 0;
    //! Snapshot of the first friendly minion summoned this combat when a
    //! first-summon copy Trinket found a full board.  The printed effect says
    //! "the first minion" and must not silently switch to a later summon when
    //! space opens.  This is transient combat state and is cleared at combat
    //! start alongside triggerProgress.
    std::optional<Minion> pendingFirstSummon;
};

//! Result of resolving one friendly combat death against Avenge Trinkets.
//! The damage flag is kept separate from the stat delta because only
//! Gilnean Thorned Rose deals damage; other AVENGE_MINION_STATS Trinkets
//! (including Bird Feeder) must never inherit that side effect.
struct TrinketAvengeResult
{
    std::int32_t attack = 0;
    std::int32_t health = 0;
    bool dealDamage = false;
    std::int32_t summonBeetles = 0;
    // Count triggered Horn instances rather than using a flag: multiple
    // copies can independently satisfy Avenge on the same death boundary.
    std::int32_t transferRightmostAttackToDragon = 0;
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
    TARGET_OR_ALL_STATS,
    DISCOVER_TIER_MINION_OR_SPELL,
    BLOOD_GEM_CHOOSE_ONE
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
    std::uint32_t legalTargetMask = 0;
    std::array<std::uint64_t, 7> legalTargetEntityIDs{};
    // False selects a recruit-board target; true selects a Tavern-slot
    // target.  Both use stable entity IDs so a refresh/mutation cannot make
    // a pending Puzzle Box choice resolve against the wrong card.
    bool targetShop = false;
    // Number of additional resolutions armed by repeat Trinkets after the
    // player commits this modal.  Keeping this on the modal preserves the
    // selected stable target/branch across the asynchronous choice.
    std::uint8_t extraResolutionCount = 0;
    // Lovely Locket is separate from Cathedral/Sushi repeats.  It survives
    // an asynchronous Choose-One decision and resolves onto another live
    // friendly minion when the branch is committed.
    std::uint8_t friendlySpellRepeatCount = 0;
};
//! A generated Mycologist token may carry a Tavern spell.  Keep its source
//! identity while the free cast is being resolved so a future target/modal
//! continuation cannot be replayed against another token.
struct Season14PendingTaughtSpell {
    bool pending = false;
    std::uint64_t sourceEntityID = 0;
    std::int32_t spellDbfID = 0;
};
//! Two-stage transform modal state. Stage one selects a stable board entity;
//! stage two exposes only active normal minions from the next Tavern tier.
enum class Season14TransformStage : std::uint8_t { NONE, TARGET, CANDIDATE };
struct Season14TransformState {
    Season14TransformStage stage = Season14TransformStage::NONE;
    std::uint64_t sourceEntityID = 0;
    std::int32_t sourceCardDbfID = 0;
    std::uint64_t targetEntityID = 0;
    std::int32_t targetIndex = -1;
    std::int32_t targetTier = 0;
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
    struct FlightpathState {
        std::int32_t pathDbfID = 0;
        std::int32_t turnsRemaining = 0;
        std::int32_t completedDbfID = 0;
    };
    FlightpathState flightpath;
    bool SelectFlightpath(std::int32_t dbfID) noexcept {
        switch (dbfID) {
            case 75704: flightpath = {dbfID, 1, 0}; return true;
            case 75705: flightpath = {dbfID, 2, 0}; return true;
            case 75706: flightpath = {dbfID, 3, 0}; return true;
            default: return false;
        }
    }
    bool AdvanceFlightpath() noexcept {
        if (flightpath.pathDbfID == 0 || flightpath.turnsRemaining <= 0) return false;
        --flightpath.turnsRemaining;
        if (flightpath.turnsRemaining != 0) return false;
        flightpath.completedDbfID = flightpath.pathDbfID;
        flightpath.pathDbfID = 0;
        flightpath.turnsRemaining = 0;
        return flightpath.completedDbfID != 0;
    }
    std::int32_t TakeCompletedFlightpath() noexcept {
        const auto completed = flightpath.completedDbfID;
        flightpath.completedDbfID = 0;
        return completed;
    }
    Season14Decision pendingDecision = Season14Decision::NONE;
    //! Remaining Darkmoon Prize selections for Ticket Collector (0/1/2).
    std::int32_t buddyTicketRemaining = 0;
    //! Reliquary Attendant copy allowance consumed this recruit turn.
    std::int32_t buddyReliquaryCopiesUsed = 0;
    std::int32_t buddyReliquaryCopiesTurn = -1;
    // Detective for Hire uses hidden opponent information; retain only the
    // committed result for Watfin, never the opponent's unrevealed board.
    std::int32_t detectiveLastGuessDbfID = 0;
    bool detectiveGuessCorrect = false;
    std::int32_t zippersPendingCards = 0;
    // Book of Medivh may expose two consecutive Tavern-spell Discovers.
    std::int32_t bookOfMedivhRemaining = 0;
    std::int32_t convictionAttackBonus = 0;
    std::int32_t convictionHealthBonus = 0;
    std::int32_t convictionExtraTargets = 0;
    std::int32_t convictionPendingImprovements = 0;
    std::int32_t buddyCombatKillHealth = 0;
    // Friendly combat-kill progress is reset at COMBAT_START.  This is
    // player-owned state; it is never reconstructed from hidden opponent data.
    std::int32_t combatKillProgress = 0;
    bool combatKillThresholdTriggered = false;
    std::int32_t azsharaWarbandAttack = 0;
    bool azsharaConquestStarted = false;
    bool expeditionFirstTurnSkipped = false;
    bool expeditionSkipFirstRecruitTurn = false;
    std::uint8_t expeditionDiscoverMask = 0;
    std::array<std::int32_t, 3> expeditionRewardDbfIDs{};
    //! Heroic Inspiration counts friendly attack declarations across combats.
    //! Once 15 are reached the counter stays complete while the generated
    //! Triple Reward waits for hand space (the reward is not burned).
    std::uint8_t heroicInspirationAttacks = 0;
    bool heroicInspirationRewardPending = false;
    std::int32_t buddyAvengeDeaths = 0;
    // Loyal Henchman counts confirmed enemy kills during the current combat.
    // This is reset at COMBAT_START and is intentionally separate from the
    // other combat-kill counters (which have different trigger semantics).
    std::int32_t loyalHenchmanKills = 0;
    std::int32_t broodmotherAvengeDeaths = 0;
    std::int32_t broodmotherWhelpBonus = 0;
    std::optional<Minion> lockAndLoadProjectile;
    void ResetBroodmotherAvenge() noexcept { broodmotherAvengeDeaths = 0; }
    bool AdvanceBroodmotherAvenge() noexcept
    { return ++broodmotherAvengeDeaths >= 4 ? (broodmotherAvengeDeaths = 0, true) : false; }
    void ImproveBroodmotherWhelp() noexcept { ++broodmotherWhelpBonus; }
    void ImproveConviction(std::int32_t choice) noexcept { if (choice == 0) ++convictionAttackBonus; else if (choice == 1) ++convictionHealthBonus; else ++convictionExtraTargets; }
    std::int32_t ConvictionAttackBonus() const noexcept { return convictionAttackBonus; }
    std::int32_t ConvictionHealthBonus() const noexcept { return convictionHealthBonus; }
    std::int32_t ConvictionExtraTargets() const noexcept { return convictionExtraTargets; }
    void QueueConvictionImprovements(std::int32_t count = 1) noexcept { convictionPendingImprovements += std::max<std::int32_t>(0, count); }
    std::int32_t PendingConvictionImprovements() const noexcept { return convictionPendingImprovements; }
    void QueueBuddyCombatKillHealth(std::int32_t amount) noexcept { buddyCombatKillHealth += std::max<std::int32_t>(0, amount); }
    std::int32_t TakeBuddyCombatKillHealth() noexcept { const auto value = buddyCombatKillHealth; buddyCombatKillHealth = 0; return value; }
    bool AdvanceLoyalHenchmanKill() noexcept { return ++loyalHenchmanKills == 2; }
    bool BeginConvictionImprovementChoice();
    bool ApplyConvictionImprovement(std::size_t offeringIndex);
    void ArmZippersCards(std::int32_t count) noexcept { zippersPendingCards += std::max(0, count); }
    std::int32_t ConsumeZippersCards() noexcept { const auto n = zippersPendingCards; zippersPendingCards = 0; return n; }
    bool ConsumeDetectiveGuessResult() noexcept { const bool r = detectiveGuessCorrect; detectiveGuessCorrect = false; detectiveLastGuessDbfID = 0; return r; }
    std::vector<Season14Offering> choiceOfferings;
    std::vector<Season14Offering> pendingOfferings;
    std::uint64_t pendingGoldenizeSourceEntityID = 0;
    std::uint64_t pendingGoldenizeFirstTargetEntityID = 0;
    std::vector<std::uint64_t> pendingGoldenizeTargets;
    std::uint64_t pendingDemonDiscoverSourceEntityID = 0;
    std::int32_t pendingDemonDiscoverRemaining = 0;
    std::uint64_t pendingUndeadDiscoverSourceEntityID = 0;
    std::int32_t pendingUndeadDiscoverRemaining = 0;
    //! Golden Primalfin Lookout reopens a second public Murloc Discover only
    //! after the first choice commits.  Keep the source entity stable across
    //! the asynchronous modal so hand-cap and replay validation remain exact.
    std::int32_t pendingPrimalfinDiscoverRemaining = 0;
    std::uint64_t pendingPrimalfinDiscoverSourceEntityID = 0;
    //! Ancient Wishbone replay state for hero powers that open sequential
    //! Discover modals.  The source DBF and pool discriminator are retained
    //! until every selected card has been materialized.
    std::int32_t pendingHeroPowerReplayDbfID = 0;
    std::int32_t pendingHeroPowerReplayRemaining = 0;
    std::int32_t pendingHeroPowerReplayTier = 0;
    std::int32_t pendingHeroPowerReplayRace = 0;
    std::uint64_t pendingMechMagnetizeSourceEntityID = 0;
    std::uint64_t pendingMechMagnetizeTargetEntityID = 0;
    std::int32_t pendingMechMagnetizeRemaining = 0;
    //! Electromagnetic Device keeps one or more Discover-to-hand choices
    //! alive across sequential modal resolutions.  This is separate from
    //! Clunker Junker's attachment Discover because the selected card is
    //! materialized in hand rather than magnetized immediately.
    std::int32_t pendingElectromagneticDiscoverRemaining = 0;
    std::int32_t pendingElectromagneticDiscoverSourceCardDbfID = 0;
    //! Phyresz's sale Discover is replayed once more for its golden copy.
    std::int32_t pendingUniqueDiscoverRemaining = 0;
    std::int32_t pendingUniqueDiscoverSourceCardDbfID = 0;
    //! Identity of the effect that created the public offering. These fields
    //! make a pending modal replayable and prevent callers from treating an
    //! offering as an anonymous global random result.
    std::uint64_t pendingSourceEntityID = 0;
    std::int32_t pendingSourceCardDbfID = 0;
    //! Windfall Tornado's sold-instance stats survive the public Discover
    //! modal; golden Windfall keeps one additional sequential choice queued.
    std::int32_t windfallAttack = 0;
    std::int32_t windfallHealth = 0;
    std::int32_t windfallRemaining = 0;
    //! Tavern slot selected by Galakrond's Greed while its replacement
    //! Discover modal is pending; -1 means no Tavern replacement is active.
    std::int32_t pendingTavernReplacementSlot = -1;
    std::int32_t pendingTavernReplacementTier = 0;
    Season14ChooseOneState chooseOne;
    //! Persistent global modifier installed by Trailblazer Sticker.
    bool trailblazerCombinedChooseOne = false;
    Season14SpellModalState spellModal;
    Season14PendingTaughtSpell pendingTaughtSpell;
    Season14TransformState transformModal;
    std::vector<Season14PersistentEffect> trinkets;
    //! Safety Patch's Ice Block immunity lasts through the current damage
    //! window/turn after it prevents a lethal hit. It is reset at the next
    //! recruit/combat start and is deliberately separate from Trinket liveness.
    bool iceBlockImmune = false;
    std::vector<Season14PersistentEffect> darkGifts;
    //! Generated quest-reward choices selected from a public modal. The
    //! reward DBF is retained until its effect family is resolved by the
    //! corresponding reward subsystem, making replay state explicit.
    std::vector<std::int32_t> generatedQuestRewards;
    //! Executable subset of generated quest rewards.  These flags are
    //! installed only by the typed ApplyChoice dispatch; raw selected DBFs
    //! never imply behavior by themselves.
    bool generatedRewardStolenGold = false;
    bool generatedRewardParasol = false;
    bool generatedRewardMirrorShield = false;
    std::int32_t generatedRewardGlobalAttack = 0;
    bool generatedRewardEvilTwin = false;
    bool generatedRewardRitualDagger = false;
    bool generatedRewardRitualDaggerRepeat = false;
    bool generatedRewardSnickerSnacks = false;
    bool generatedRewardExquisiteConch = false;
    bool generatedRewardConchUsedThisTurn = false;
    bool generatedRewardSecretSinstone = false;
    bool generatedRewardRedHand = false;
    bool generatedRewardTinyHenchmen = false;
    bool generatedRewardStaffOfOrigination = false;
    // Additional generated quest-reward families. These are player-owned
    // counters/auras, so replaying a selected DBF cannot accidentally infer
    // an effect without the typed lifecycle installation below.
    bool generatedRewardCookedBook = false;
    std::int32_t generatedRewardCookedBookBonus = 1;
    bool generatedRewardTealTiger = false;
    std::int32_t generatedRewardRefreshesThisTurn = 0;
    bool generatedRewardAlterEgo = false;
    bool generatedRewardAlterEgoEven = true;
    bool generatedRewardMenagerieMayhem = false;
    bool generatedRewardHiddenVault = false;
    std::int32_t generatedRewardHiddenVaultGold = 1;
    bool generatedRewardVolatileVenom = false;
    bool generatedRewardBloodGoblet = false;
    bool generatedRewardSinfallMedallion = false;
    std::int32_t generatedRewardSinfallTier = 0;
    std::uint64_t generatedRewardSinfallSourceEntityID = 0;
    bool generatedRewardAnimaBribe = false;
    bool generatedRewardVictimsSpecter = false;
    bool generatedRewardDevilsInDetails = false;
    bool generatedRewardPilferedLamps = false;
    bool generatedRewardKidnapSack = false;
    bool generatedRewardAnotherHiddenBody = false;
    //! Ethereal Evidence is a one-shot reward replacement.  The pending
    //! modal itself remains in the normal decision state so replay captures
    //! its exact two DBF offerings rather than a random host-side choice.
    bool generatedRewardEtherealEvidence = false;
    bool generatedRewardGhastlyMask = false;
    //! Exact pinned minion selected for Ghastly Mask's {0} card.  The DBF is
    //! retained after delivery so replay cannot reroll the linked entity.
    std::int32_t generatedRewardGhastlyCardDbfID = 0;
    bool generatedRewardGhastlyCardDelivered = false;
    bool generatedRewardUnmurloc = false;
    //! Pinned hero DBF selected by Un-Murloc Your Potential.  The paired
    //! hero-power DBF is validated against the same manifest pair at apply.
    std::int32_t generatedRewardUnmurlocHeroDbfID = 0;
    //! Friends Along the Way chooses the lobby's excluded-race complement
    //! once when the reward is installed.  Persisting the race makes replay
    //! and repeated lifecycle callbacks deterministic.
    Race generatedRewardFriendsRace = Race::INVALID;
    bool generatedRewardPartnerInCrime = false;
    bool generatedRewardWisdomball = false;
    bool generatedRewardWisdomballUsedThisTurn = false;
    bool generatedRewardEssenceOfZerus = false;
    bool generatedRewardEnhanceAMatic = false;
    bool generatedRewardGoldenHammer = false;
    bool generatedRewardSturdyShard = false;
    bool generatedRewardBloodsoakedTome = false;
    bool generatedRewardEndlessBloodMoon = false;
    bool generatedRewardBeyondTheMirage = false;
    bool generatedRewardInvigoratingConch = false;
    bool generatedRewardTimelineAcceleration = false;
    bool generatedRewardSmeltingChamber = false;
    std::int32_t generatedRewardSmeltingTier = 1;
    bool generatedRewardStashOfTheScribe = false;
    // Generated reward passives whose trigger is a successful purchase/cast.
    bool generatedRewardSplittingScroll = false;
    bool generatedRewardDoubleHeaded = false;
    bool generatedRewardDoubleHeadedUsedThisTurn = false;
    std::int32_t generatedRewardBoomSquadDeaths = 0;
    bool generatedRewardBoomSquad = false;
    //! Avenge counters for the generated Tavern-reward families that resolve
    //! from the combat death lifecycle rather than a CardDef trigger.
    std::int32_t generatedRewardCycleEnergyDeaths = 0;
    std::int32_t generatedRewardStableAmalgamationDeaths = 0;
    bool generatedRewardCycleEnergy = false;
    bool generatedRewardStableAmalgamation = false;
    bool generatedRewardTurbulentTombs = false;
    bool generatedRewardMapUnknown = false;
    bool generatedRewardTemporalTampering = false;
    bool generatedRewardTemporalTamperingReentry = false;
    bool generatedRewardNineLives = false;
    bool generatedRewardTotemicTavern = false;
    bool generatedRewardPurifiedShard = false;
    bool generatedRewardTheWall = false;
    bool generatedRewardBattlecryRepeat = false;
    bool generatedRewardAvengeRefresh = false;
    bool generatedRewardStartTurnRandomSpells = false;
    bool generatedRewardScepterOfGuidance = false;
    bool generatedRewardGoldenKobold = false;
    std::int32_t generatedRewardGoldenKoboldRefreshes = 0;
    bool generatedRewardSecretCulprit = false;
    bool generatedRewardDoppelgangersLocket = false;
    bool generatedRewardTumblingDisaster = false;
    std::int32_t generatedRewardTumblingAvenge = 0;
    std::int32_t generatedRewardTumblingBonus = 4;
    bool generatedRewardOpenAuditions = false;
    bool generatedRewardRighteousCharge = false;
    bool generatedRewardRushingWinds = false;
    bool generatedRewardNorgannon = false;
    bool generatedRewardMagicfin = false;
    bool generatedRewardUntoldRiches = false;
    bool generatedRewardGoldenForge = false;
    bool generatedRewardQuaintBoutique = false;
    bool generatedRewardJumboWarehouse = false;
    bool generatedRewardCosmicReward = false;
    //! Perpetual Incantation improves every subsequently resolved Tavern
    //! spell by +2/+1, with no trigger cap.
    bool generatedRewardPerpetualIncantation = false;
    //! Rallying Cry causes each Rally trigger to resolve one additional time.
    bool generatedRewardRallyingCry = false;
    bool generatedRewardRallyingCryResolving = false;
    //! No Place Like Holmes opens a public guess modal from the last observed
    //! opposing warband; keep it armed until a valid observation exists.
    bool generatedRewardOpponentWarbandGuess = false;
    std::int32_t generatedRewardAvengeRefreshDeaths = 0;
    // Untamed Sorcery may pause on a public target modal.  Keep the number
    // of unresolved casts so selecting a target resumes the same five-cast
    // sequence instead of silently ending the reward early.
    std::int32_t generatedRewardRandomSpellsRemaining = 0;
    //! Lavish Cape may pause after selecting a targeted random Tavern spell.
    //! Keep the remaining casts separate from generated quest rewards so a
    //! target modal can resume the Cape sequence without conflating sources.
    std::int32_t lavishCapeRandomSpellsRemaining = 0;

    //! Install one supported generated quest reward effect. Returns false
    //! for metadata-only choices so callers cannot award executable credit.
    bool ApplyGeneratedQuestReward(std::int32_t dbfID) noexcept;
    bool HasGeneratedRewardStolenGold() const noexcept { return generatedRewardStolenGold; }
    bool HasGeneratedRewardParasol() const noexcept { return generatedRewardParasol; }
    bool HasGeneratedRewardMirrorShield() const noexcept { return generatedRewardMirrorShield; }
    bool HasGeneratedRewardEvilTwin() const noexcept { return generatedRewardEvilTwin; }
    bool HasGeneratedRewardRitualDagger() const noexcept { return generatedRewardRitualDagger; }
    bool HasGeneratedRewardRitualDaggerRepeat() const noexcept { return generatedRewardRitualDaggerRepeat; }
    bool HasGeneratedRewardSnickerSnacks() const noexcept { return generatedRewardSnickerSnacks; }
    bool HasGeneratedRewardExquisiteConch() const noexcept { return generatedRewardExquisiteConch; }
    bool HasGeneratedRewardSecretSinstone() const noexcept { return generatedRewardSecretSinstone; }
    bool HasGeneratedRewardRedHand() const noexcept { return generatedRewardRedHand; }
    bool HasGeneratedRewardTinyHenchmen() const noexcept { return generatedRewardTinyHenchmen; }
    bool HasGeneratedRewardStaffOfOrigination() const noexcept { return generatedRewardStaffOfOrigination; }
    bool HasGeneratedRewardCookedBook() const noexcept { return generatedRewardCookedBook; }
    bool HasGeneratedRewardTealTiger() const noexcept { return generatedRewardTealTiger; }
    bool HasGeneratedRewardAlterEgo() const noexcept { return generatedRewardAlterEgo; }
    bool HasGeneratedRewardMenagerieMayhem() const noexcept { return generatedRewardMenagerieMayhem; }
    bool HasGeneratedRewardHiddenVault() const noexcept { return generatedRewardHiddenVault; }
    bool HasGeneratedRewardVolatileVenom() const noexcept { return generatedRewardVolatileVenom; }
    bool HasGeneratedRewardBloodGoblet() const noexcept { return generatedRewardBloodGoblet; }
    bool HasGeneratedRewardSinfallMedallion() const noexcept { return generatedRewardSinfallMedallion; }
    bool HasGeneratedRewardAnimaBribe() const noexcept { return generatedRewardAnimaBribe; }
    bool HasGeneratedRewardVictimsSpecter() const noexcept { return generatedRewardVictimsSpecter; }
    bool HasGeneratedRewardDevilsInDetails() const noexcept { return generatedRewardDevilsInDetails; }
    bool HasGeneratedRewardPilferedLamps() const noexcept { return generatedRewardPilferedLamps; }
    bool HasGeneratedRewardKidnapSack() const noexcept { return generatedRewardKidnapSack; }
    bool HasGeneratedRewardAnotherHiddenBody() const noexcept { return generatedRewardAnotherHiddenBody; }
    bool HasGeneratedRewardEtherealEvidence() const noexcept { return generatedRewardEtherealEvidence; }
    bool HasGeneratedRewardGhastlyMask() const noexcept { return generatedRewardGhastlyMask; }
    std::int32_t GeneratedRewardGhastlyCardDbfID() const noexcept { return generatedRewardGhastlyCardDbfID; }
    void SetGeneratedRewardGhastlyCardDbfID(std::int32_t dbfID) noexcept { generatedRewardGhastlyCardDbfID = dbfID; }
    bool GhastlyCardDelivered() const noexcept { return generatedRewardGhastlyCardDelivered; }
    void MarkGhastlyCardDelivered() noexcept { generatedRewardGhastlyCardDelivered = true; }
    bool HasGeneratedRewardUnmurloc() const noexcept { return generatedRewardUnmurloc; }
    std::int32_t GeneratedRewardUnmurlocHeroDbfID() const noexcept { return generatedRewardUnmurlocHeroDbfID; }
    void SetGeneratedRewardUnmurloc(std::int32_t heroDbfID) noexcept {
        generatedRewardUnmurloc = true;
        generatedRewardUnmurlocHeroDbfID = heroDbfID;
    }
    Race GeneratedRewardFriendsRace() const noexcept { return generatedRewardFriendsRace; }
    void SetGeneratedRewardFriendsRace(Race race) noexcept { generatedRewardFriendsRace = race; }
    bool ConsumeGeneratedRewardConch() noexcept {
        if (!generatedRewardExquisiteConch || generatedRewardConchUsedThisTurn) return false;
        generatedRewardConchUsedThisTurn = true;
        return true;
    }
    std::int32_t GeneratedRewardGlobalAttack() const noexcept { return generatedRewardGlobalAttack; }
    bool HasGeneratedRewardPartnerInCrime() const noexcept { return generatedRewardPartnerInCrime; }
    bool HasGeneratedRewardWisdomball() const noexcept { return generatedRewardWisdomball; }
    bool HasGeneratedRewardEssenceOfZerus() const noexcept { return generatedRewardEssenceOfZerus; }
    bool HasGeneratedRewardEnhanceAMatic() const noexcept { return generatedRewardEnhanceAMatic; }
    bool HasGeneratedRewardGoldenHammer() const noexcept { return generatedRewardGoldenHammer; }
    bool HasGeneratedRewardSturdyShard() const noexcept { return generatedRewardSturdyShard; }
    bool HasGeneratedRewardBloodsoakedTome() const noexcept { return generatedRewardBloodsoakedTome; }
    bool HasGeneratedRewardEndlessBloodMoon() const noexcept { return generatedRewardEndlessBloodMoon; }
    bool HasGeneratedRewardBeyondTheMirage() const noexcept { return generatedRewardBeyondTheMirage; }
    bool HasGeneratedRewardInvigoratingConch() const noexcept { return generatedRewardInvigoratingConch; }
    bool HasGeneratedRewardTimelineAcceleration() const noexcept { return generatedRewardTimelineAcceleration; }
    bool HasGeneratedRewardSmeltingChamber() const noexcept { return generatedRewardSmeltingChamber; }
    std::int32_t GeneratedRewardSmeltingTier() const noexcept { return generatedRewardSmeltingTier; }
    void AdvanceGeneratedRewardSmeltingTier() noexcept { if (generatedRewardSmeltingTier < 6) ++generatedRewardSmeltingTier; }
    bool HasGeneratedRewardStashOfTheScribe() const noexcept { return generatedRewardStashOfTheScribe; }
    bool HasGeneratedRewardSplittingScroll() const noexcept { return generatedRewardSplittingScroll; }
    bool HasGeneratedRewardDoubleHeaded() const noexcept { return generatedRewardDoubleHeaded; }
    bool HasGeneratedRewardBoomSquad() const noexcept { return generatedRewardBoomSquad; }
    bool HasGeneratedRewardCycleEnergy() const noexcept { return generatedRewardCycleEnergy; }
    bool HasGeneratedRewardStableAmalgamation() const noexcept { return generatedRewardStableAmalgamation; }
    bool HasGeneratedRewardTurbulentTombs() const noexcept { return generatedRewardTurbulentTombs; }
    void ResetGeneratedRewardAvenge() noexcept
    {
        generatedRewardCycleEnergyDeaths = 0;
        generatedRewardStableAmalgamationDeaths = 0;
        generatedRewardAvengeRefreshDeaths = 0;
    }
    bool HasGeneratedRewardMapUnknown() const noexcept { return generatedRewardMapUnknown; }
    bool HasGeneratedRewardTemporalTampering() const noexcept { return generatedRewardTemporalTampering; }
    bool HasGeneratedRewardNineLives() const noexcept { return generatedRewardNineLives; }
    bool HasGeneratedRewardTotemicTavern() const noexcept { return generatedRewardTotemicTavern; }
    bool HasGeneratedRewardPurifiedShard() const noexcept { return generatedRewardPurifiedShard; }
    bool HasGeneratedRewardTheWall() const noexcept { return generatedRewardTheWall; }
    bool HasGeneratedRewardBattlecryRepeat() const noexcept { return generatedRewardBattlecryRepeat; }
    bool HasGeneratedRewardAvengeRefresh() const noexcept { return generatedRewardAvengeRefresh; }
    bool HasGeneratedRewardStartTurnRandomSpells() const noexcept { return generatedRewardStartTurnRandomSpells; }
    bool HasGeneratedRewardScepterOfGuidance() const noexcept { return generatedRewardScepterOfGuidance; }
    bool HasGeneratedRewardGoldenKobold() const noexcept { return generatedRewardGoldenKobold; }
    bool HasGeneratedRewardSecretCulprit() const noexcept { return generatedRewardSecretCulprit; }
    bool HasGeneratedRewardDoppelgangersLocket() const noexcept { return generatedRewardDoppelgangersLocket; }
    bool HasGeneratedRewardTumblingDisaster() const noexcept { return generatedRewardTumblingDisaster; }
    bool HasGeneratedRewardOpenAuditions() const noexcept { return generatedRewardOpenAuditions; }
    bool HasGeneratedRewardRighteousCharge() const noexcept { return generatedRewardRighteousCharge; }
    bool HasGeneratedRewardRushingWinds() const noexcept { return generatedRewardRushingWinds; }
    bool HasGeneratedRewardNorgannon() const noexcept { return generatedRewardNorgannon; }
    bool HasGeneratedRewardMagicfin() const noexcept { return generatedRewardMagicfin; }
    bool HasGeneratedRewardUntoldRiches() const noexcept { return generatedRewardUntoldRiches; }
    bool HasGeneratedRewardGoldenForge() const noexcept { return generatedRewardGoldenForge; }
    bool HasGeneratedRewardPerpetualIncantation() const noexcept { return generatedRewardPerpetualIncantation; }
    bool HasGeneratedRewardRallyingCry() const noexcept { return generatedRewardRallyingCry; }
    bool HasGeneratedRewardOpponentWarbandGuess() const noexcept { return generatedRewardOpponentWarbandGuess; }
    bool HasGeneratedRewardQuaintBoutique() const noexcept { return generatedRewardQuaintBoutique; }
    bool HasGeneratedRewardJumboWarehouse() const noexcept { return generatedRewardJumboWarehouse; }
    bool HasGeneratedRewardCosmicReward() const noexcept { return generatedRewardCosmicReward; }
    std::int32_t GeneratedRewardTumblingAvenge() const noexcept { return generatedRewardTumblingAvenge; }
    std::int32_t GeneratedRewardTumblingBonus() const noexcept { return generatedRewardTumblingBonus; }
    void ResetGeneratedRewardTumblingAvenge() noexcept { generatedRewardTumblingAvenge = 0; }
    bool AdvanceGeneratedRewardTumblingAvenge() noexcept
    {
        if (!generatedRewardTumblingDisaster || ++generatedRewardTumblingAvenge < 4)
            return false;
        generatedRewardTumblingAvenge = 0;
        generatedRewardTumblingBonus += 4;
        return true;
    }
    void RecordGeneratedRewardGoldenKoboldRefresh() noexcept
    {
        if (generatedRewardGoldenKobold) ++generatedRewardGoldenKoboldRefreshes;
    }
    std::int32_t GeneratedRewardGoldenKoboldRefreshes() const noexcept
    {
        return generatedRewardGoldenKoboldRefreshes;
    }
    bool ConsumeGeneratedRewardGoldenKoboldTrigger() noexcept
    {
        if (!generatedRewardGoldenKobold || generatedRewardGoldenKoboldRefreshes < 5)
            return false;
        generatedRewardGoldenKoboldRefreshes -= 5;
        return true;
    }
    void BeginGeneratedRewardRandomSpells(std::int32_t amount) noexcept
    {
        generatedRewardRandomSpellsRemaining = std::max<std::int32_t>(0, amount);
    }
    std::int32_t GeneratedRewardRandomSpellsRemaining() const noexcept
    {
        return generatedRewardRandomSpellsRemaining;
    }
    void SetGeneratedRewardRandomSpellsRemaining(std::int32_t amount) noexcept
    {
        generatedRewardRandomSpellsRemaining = std::max<std::int32_t>(0, amount);
    }
    bool HasPendingGeneratedRewardRandomSpells() const noexcept
    {
        return generatedRewardRandomSpellsRemaining > 0;
    }
    void FinishGeneratedRewardRandomSpells() noexcept
    {
        generatedRewardRandomSpellsRemaining = 0;
    }
    void BeginLavishCapeRandomSpells(std::int32_t amount) noexcept
    {
        lavishCapeRandomSpellsRemaining = std::max<std::int32_t>(0, amount);
    }
    std::int32_t LavishCapeRandomSpellsRemaining() const noexcept
    {
        return lavishCapeRandomSpellsRemaining;
    }
    void SetLavishCapeRandomSpellsRemaining(std::int32_t amount) noexcept
    {
        lavishCapeRandomSpellsRemaining = std::max<std::int32_t>(0, amount);
    }
    bool HasPendingLavishCapeRandomSpells() const noexcept
    {
        return lavishCapeRandomSpellsRemaining > 0;
    }
    void FinishLavishCapeRandomSpells() noexcept
    {
        lavishCapeRandomSpellsRemaining = 0;
    }
    bool AdvanceGeneratedRewardAvengeRefresh() noexcept
    {
        return generatedRewardAvengeRefresh &&
               ++generatedRewardAvengeRefreshDeaths >= 2
                   ? (generatedRewardAvengeRefreshDeaths = 0, true)
                   : false;
    }

    std::int32_t heroPowerDbfID = 0;
    std::int32_t heroPowerCost = 0;
    //! The Perfect Crime's persistent one-gold-per-recruit discount.  It is
    //! deliberately separate from the one-shot Galaxy's Lens discount.
    std::int32_t perfectCrimeDiscount = 0;
    //! Public information retained from the opponent's most recent combat;
    //! Detective for Hire only receives this through its two-card choice.
    std::vector<std::int32_t> lastOpponentCombatMinionDbfIDs;
    std::vector<Minion> lastOpponentCombatMinionSnapshots;
    //! Public hero linkage observed for the most recent opponent combat.
    std::int32_t lastOpponentBuddyDbfID = 0;
    std::int32_t detectiveCorrectDbfID = 0;
    bool rapidReanimationArmed = false;
    std::uint64_t rapidReanimationTargetEntityID = 0;
    std::int32_t rapidReanimationTargetSlot = -1;
    std::optional<Minion> rapidReanimationSnapshot;
    bool heroPowerAvailable = false;
    bool heroPowerUsed = false;
    //! Three Wishes is a persistent three-charge hero power.
    std::int32_t threeWishesRemaining = 3;
    std::int32_t buddyExtraHeroPowerUses = 0;
    //! Shadow Warden arms the next one/two successful targeted hero powers
    //! to make their selected minion golden.  This is player-owned state,
    //! independent of the ordinary extra-use counter.
    std::int32_t buddyGoldenHeroPowerUses = 0;
    //! Legacy Buddy lifecycle counters.  These are player-owned so combat
    //! copies cannot leak state across lobbies or recruit/combat boundaries.
    std::int32_t chromieRefreshesThisTurn = 0;
    //! Imperial Defender's once-per-turn fan-out.  This is a count rather
    //! than a boolean because multiple Buddy copies each have an independent
    //! trigger, while a golden copy still consumes only one trigger slot.
    std::int32_t imperialDefenderCopiesUsed = 0;
    void EnableBuddyExtraHeroPowerUses(std::int32_t n) noexcept { buddyExtraHeroPowerUses = std::max(buddyExtraHeroPowerUses, n); }
    void ResetBuddyExtraHeroPowerUses() noexcept { buddyExtraHeroPowerUses = 0; }
    void ArmBuddyGoldenHeroPowerUses(std::int32_t n) noexcept
    {
        if (n > 0) buddyGoldenHeroPowerUses += n;
    }
    bool ConsumeBuddyGoldenHeroPowerUse() noexcept
    {
        if (buddyGoldenHeroPowerUses <= 0) return false;
        --buddyGoldenHeroPowerUses;
        return true;
    }
    bool powerOfStormActive = false;
    std::int32_t luckyRollCooldown = 0;

    //! State for the currently implemented modern hero-power families.
    //! Keeping this with the player-owned Season 14 state makes discounts and
    //! lifecycle counters available to both the event hooks and Player's
    //! action-cost checks.
    Season14HeroPowerBatch1State heroPowerBatch1;
    Season14HeroPowerBatch2State heroPowerBatch2;
    Season14HeroPowerBatch4State heroPowerBatch4;
    Season14HeroPowerBatch5State heroPowerBatch5;
    Season14HeroPowerBatch6State heroPowerBatch6;
    Season14HeroPowerBatch7State heroPowerBatch7;
    Season14HeroPowerBatch8State heroPowerBatch8;
    Season14HeroPowerBatch9State heroPowerBatch9;
    std::array<Race, 3> stirPotRaces{};
    std::int32_t stirPotCount = 0;
    std::int32_t imprisonedSlot = -1;
    //! Stable entity identity for Imprison; the slot can move when other
    //! Tavern cards are bought or removed before the two-turn duration ends.
    std::int32_t imprisonedEntityID = -1;
    std::int32_t imprisonedTurns = 0;
    std::int32_t mechGyverDeaths = 0;
    //! Persistent +1/+1 earned by Tentacular's combat-start Tentacle per sale.
    std::int32_t tentacularBonus = 0;
    std::int32_t embraceElementDbfID = 0;
    //! Spirit Raptor payloads keyed by stable minion entity identity.
    std::vector<std::pair<std::uint64_t, std::vector<std::int32_t>>> spiritRaptorElements;
    //! Nine Frogs purchase charges keyed by the concrete Buddy entity.  The
    //! key prevents duplicate copies from sharing the printed (9 left)
    //! counter and keeps combat/sale lifecycles isolated.
    std::vector<std::pair<std::uint64_t, std::int32_t>> nineFrogsPurchasesRemaining;
    void ForgetNineFrogs(std::uint64_t entityID) noexcept
    {
        nineFrogsPurchasesRemaining.erase(
            std::remove_if(nineFrogsPurchasesRemaining.begin(),
                           nineFrogsPurchasesRemaining.end(),
                           [entityID](const auto& entry) {
                               return entry.first == entityID;
                           }),
            nineFrogsPurchasesRemaining.end());
    }
    //! Triple formation creates a fresh golden Buddy identity. Its printed
    //! counter is nine even when a normal source copy had already fired, so
    //! discard source records before arming the surviving entity.
    void ResetNineFrogs(std::uint64_t entityID, std::int32_t charges = 9)
    {
        ForgetNineFrogs(entityID);
        nineFrogsPurchasesRemaining.emplace_back(entityID, charges);
    }
    //! A Raptor's memory belongs to that concrete minion instance.  Remove
    //! it when the instance leaves the recruit board without a combat
    //! death (for example, a sale), so entity state cannot leak to a later
    //! card lifecycle.
    void ForgetSpiritRaptor(std::uint64_t entityID) noexcept
    {
        spiritRaptorElements.erase(
            std::remove_if(spiritRaptorElements.begin(),
                           spiritRaptorElements.end(),
                           [entityID](const auto& entry) {
                               return entry.first == entityID;
                           }),
            spiritRaptorElements.end());
    }
    std::int32_t murlocRewardSells = 0;
    std::int32_t murlocRewardsRemaining = 5;
    std::int32_t battlecryRewardBuys = 0;
    bool battlecryRewardGiven = false;
    //! Clockwork Assistant's golden form chains a second public Discover.
    std::uint8_t clockworkDiscoverRemaining = 0;
    bool AdvanceMechGyverDeath() noexcept
    {
        if (++mechGyverDeaths < 9) return false;
        mechGyverDeaths = 0;
        return true;
    }
    std::vector<std::string> reclaimedSoulsDeaths;
    void RecordReclaimedSoulsDeath(const Minion& minion);

    //! Batch 3 currently contains stateless combat/activation families.  It
    //! is kept as an explicit state member for schema clarity and future
    //! counters, while all unsupported families remain fail-closed.
    std::uint8_t heroPowerBatch3State = 0;
    std::int32_t recruitTurnNumber = 0;
    std::int32_t warpGateBuyCount = 0;
    std::int32_t warpGateSelectedDbfID = 0;
    std::int32_t warpGateRewardDbfID = 0;
    //! Lifetime reduction applied only to the pinned Protoss pool after a
    //! Sentry Deathrattle resolves.
    std::int32_t protossCostReduction = 0;
    std::int32_t carrierInterceptors = 0;
    //! Spawning Pool's turn-based cost reduction, capped at its printed cost.
    std::int32_t spawningPoolDiscount = 0;
    std::uint64_t spawningPoolLarvaEntityID = 0;
    bool spawningPoolUnlocked = false;
    std::uint64_t liftOffBattlecruiserEntityID = 0;
    //! Lift Off upgrade lifecycle.  Upgrades are generated by completed
    //! Tavern refreshes, bought from the Tavern spell row, and consumed by
    //! the Battlecruiser entity.  These counters are player-owned so a
    //! replayed spell/modal cannot manufacture a second upgrade.
    std::int32_t liftOffUpgradeTier = 0;
    std::int32_t liftOffUpgradesBoughtThisTurn = 0;
    bool liftOffFreeUpgradeAvailable = false;
    std::int32_t liftOffYamatoDamage = 0;
    std::int32_t liftOffRallyAttack = 0;
    std::int32_t liftOffDeathrattleAttack = 0;
    std::int32_t liftOffDeathrattleHealth = 0;
    bool liftOffFortifiedBunker = false;
    bool liftOffMissilePod = false;
    bool liftOffUltraCapacitor = false;
    //! Reserved fixed-capacity payload for delayed end-turn effects.  HP104
    //! deliberately leaves it empty: recipients are chosen at end turn.
    std::array<std::uint64_t, 32> cthunEndTurnTargets{};
    std::uint8_t cthunEndTurnTargetCount = 0;
    bool cthunEndTurnPending = false;
    std::uint8_t cthunEndTurnApplications = 0;
    std::uint8_t cthunRepeatCount = 0;
    bool tierMinionStartCombatPending = false;
    std::int32_t tierMinionStartCombatTier = 0;
    std::int32_t championRewardDbfID = 0;
    bool championRewardReady = false;

    //! Hooks for effects whose entity behavior is implemented elsewhere.
    bool lockboxActive = false;
    std::int32_t lockboxAdvance = 0;
    // Remaining recruit starts before the active Lockbox opens.  Keeping the
    // countdown in Season14State makes portrait and minion sources replayable
    // and prevents a duplicate source from creating a second box.
    std::int32_t lockboxTurnsRemaining = 0;
    bool fishbaitActive = false;
    //! DBF ID of Fishbait generated by the latest Snarky Shark sale.
    std::int32_t fishbaitDbfID = 0;
    bool pendingHandLock = false;
    //! Deferred/economy state used by simple Tavern spells.  These counters
    //! are player-owned so refreshing or replacing a Tavern cannot lose a
    //! spell's intended duration.
    std::int32_t nextTurnGold = 0;
    std::int32_t pendingCombatRewardDbfID = 0;
    std::vector<Season14PendingCombatBuff> pendingCombatBuffs;
    // Each Rally resolves against a hand snapshot independently.  Keeping a
    // queue (rather than one snapshot plus a count) is important for golden
    // Expert Aviator and for multiple Rally sources: two resolutions may pick
    // different hand instances and must not turn into duplicate copies of the
    // last one selected.
    std::vector<std::pair<Minion, std::int32_t>> pendingCombatHandSummons;
    struct PendingBloodGemGolemAttack {
        std::uint64_t targetEntityID = 0;
        std::int32_t attack = 0;
        std::int32_t health = 0;
    };
    std::vector<PendingBloodGemGolemAttack> pendingBloodGemGolemAttacks;
    //! Full combat-only snapshots retained by Stitched Salvager deathrattles.
    //! Handles are stored in TaskType to avoid recursively embedding Minion's
    //! card-power task graph inside itself.
    std::vector<std::optional<Minion>> pendingExactCopySnapshots;
    std::vector<Minion> combatDeadMinions;
    //! Boom Controller's first friendly Mech death is retained until the
    //! post-death boundary has room to summon its exact combat copy.
    std::optional<Minion> boomControllerFirstMech;
    void RecordCombatDeadMinion(const Minion& minion) { combatDeadMinions.push_back(minion); }
    //! Returns plain copies of the first matching combat deaths.  History is
    //! intentionally non-consuming: multiple Kangor deathrattles each refer
    //! to the same first deaths; it is cleared at the next combat start.
    std::vector<Minion> TakeCombatDeadMinions(Race race, std::size_t count);
    std::optional<Minion> CopyLastCombatDeadMinion() const;
    void ClearCombatDeadMinions() noexcept { combatDeadMinions.clear(); }
    void RecordBoomControllerMech(const Minion& minion)
    {
        if (!boomControllerFirstMech.has_value()) boomControllerFirstMech = minion;
    }
    const std::optional<Minion>& BoomControllerFirstMech() const noexcept
    {
        return boomControllerFirstMech;
    }
    void ClearBoomControllerMech() noexcept { boomControllerFirstMech.reset(); }
    std::size_t CountCombatDeadMinions(Race race) const noexcept
    {
        return static_cast<std::size_t>(std::count_if(
            combatDeadMinions.begin(), combatDeadMinions.end(),
            [race](const Minion& minion) { return minion.HasRace(race); }));
    }
    std::vector<std::int32_t> pendingCombatStartEffects;
    //! Remaining Discover selections for a multi-spell Activate.
    std::int32_t tavernSpellDiscoverRemaining = 0;
    //! Additional resolutions queued by Cathedral/Sushi for an ordinary
    //! Tavern-spell Discover.  Discover is asynchronous, so the replay must
    //! retain the source and (for typed Discover spells) the stable target
    //! until the public offering is committed.
    std::int32_t discoverReplayRemaining = 0;
    std::int32_t discoverReplaySourceSpellDbfID = 0;
    std::uint64_t discoverReplayTargetEntityID = 0;
    bool shopBloodGemsOnRefresh = false;
    //! One-shot tribe filter consumed by the next Tavern fill.
    Race pendingRefreshRace = Race::INVALID;
    //! Number of BG28_884 casts waiting for the same upcoming combat.
    std::int32_t pendingCombatRewardCount = 0;
    bool firstMinionPlayedThisTurn = false;
    std::int32_t battlecryBuysThisTurn = 0;
    //! Override the base Tavern minion purchase cost for the current recruit
    //! turn; -1 means normal tier-independent cost handling.
    std::int32_t temporaryMinionPurchaseCost = -1;
    std::int32_t minionsPlayedThisTurn = 0;
    //! Card identities played this recruit turn, used by Jandice's
    //! Apprentice.  This is deliberately reset at the recruit boundary and
    //! is not a lifetime/stat counter.
    std::vector<std::string> repeatedPlayCardIDs;
    std::int32_t progressiveAvengeAttack = 1;
    std::int32_t progressiveAvengeHealth = 1;
    std::int32_t battlecriesTriggered = 0;
    std::int32_t deathrattlesTriggered = 0;
    std::int32_t maxGoldDelta = 0;
    std::int32_t freeRefreshes = 0;
    std::int32_t persistentShopAttack = 0;
    std::int32_t persistentShopHealth = 0;
    bool lastCombatLost = false;
    //! Permanent all-minion aura installed by deterministic Trinkets.
    std::int32_t persistentMinionAttack = 0;
    std::int32_t persistentMinionHealth = 0;
    std::int32_t persistentFodderAttack = 0;
    std::int32_t persistentFodderHealth = 0;
    std::int32_t soldMinionsThisTurn = 0;
    std::int32_t trinketStatSpellDiscount = 0;
    std::int32_t trinketFreeSpellUses = 0;
    std::int32_t trinketFreeSpellUsesPerTurn = 0;
    std::int32_t temporaryRefreshShopAttack = 0;
    std::int32_t temporaryRefreshShopHealth = 0;
    void AddTemporaryRefreshShopStats(std::int32_t attack,
                                      std::int32_t health) noexcept
    { temporaryRefreshShopAttack += attack; temporaryRefreshShopHealth += health; }
    std::int32_t refreshShopStatsDeltaAttack = 0;
    std::int32_t refreshShopStatsDeltaHealth = 0;
    //! Permanent upgrade-cost reduction earned by successful refreshes.
    std::int32_t refreshUpgradeCostDiscount = 0;
    std::int32_t temporaryTavernSpellAttack = 0;
    std::int32_t temporaryTavernSpellHealth = 0;
    //! Cumulative improvement applied to newly created Tasty Lobsters.  This
    //! is player-owned game state, so it survives combat, deaths, Tavern
    //! refreshes, and recruit-phase transitions.
    std::int32_t futureLobsterAttack = 0;
    std::int32_t futureLobsterHealth = 0;
    //! Cumulative per-cast improvement applied to future Deep Blues
    //! Spellcraft cards.  This is player-owned state and survives refresh,
    //! combat, and recruit-phase transitions.
    std::int32_t futureDeepBluesAttack = 0;
    std::int32_t futureDeepBluesHealth = 0;
    std::int32_t futureBallerAttack = 0;
    std::int32_t futureBallerHealth = 0;
    std::int32_t trinketExtraShopSlots = 0;
    std::int32_t magneticMechPurchaseCostDiscount = 0;
    // Some Trinkets set a target Tavern size rather than adding a flat
    // number of offers (Felbat Portrait keeps seven offers at every tier).
    std::int32_t trinketMinimumShopSlots = 0;
    std::int32_t refreshExtraShopSlots = 0;
    // True only while a player-initiated Tavern refresh is filling offers;
    // refresh-triggered hero powers must not apply during turn-start setup.
    bool refreshInProgress = false;
    std::int32_t spellMinionAttackProgress = 0;
    //! Number of successfully resolved spells this game.
    std::int32_t successfulSpellCount = 0;
    // Snapshot used by end-of-turn Trinkets whose payload scales with the
    // number of Tavern spells cast during the current recruit turn.
    std::int32_t successfulSpellCountAtRecruitStart = 0;
    //! Rewards produced by spell-count Trinkets and not yet delivered by
    //! Player. Hand rewards remain pending when the hand is full.
    std::int32_t pendingSpellCountNagaRewards = 0;
    std::int32_t pendingSpellCountGold = 0;
    //! Number of Blood Gems to play on each friendly minion after
    //! thresholded spell-count Trinkets resolve. This is a pending board
    //! effect rather than a hand reward, so it is independent of hand cap.
    std::int32_t pendingSpellCountBloodGems = 0;
    std::vector<std::int32_t> distinctSpellsThisTurn;
    void RecordDistinctSpell(std::int32_t dbfID) {
        if (dbfID > 0 && std::find(distinctSpellsThisTurn.begin(), distinctSpellsThisTurn.end(), dbfID) == distinctSpellsThisTurn.end()) distinctSpellsThisTurn.push_back(dbfID);
    }
    void ResetDistinctSpells() noexcept { distinctSpellsThisTurn.clear(); }
    std::size_t DistinctSpellsThisTurn() const noexcept { return distinctSpellsThisTurn.size(); }
    std::int32_t spellMinionAttackDelta = 0;
    bool tierSixGoldClaimed = false;
    std::int32_t treasureMapTurns = 0;
    bool treasureMapClaimed = false;
    std::int32_t spellCastMinionAttackDelta = 0;
    std::int32_t spellCastMinionHealthDelta = 0;
    std::int32_t trinketHigherTierRefreshes = 0;
    //! Number of pending refresh fills whose fresh offers must all be Tier 6.
    std::int32_t trinketTierSixOnlyRefreshes = 0;
    std::int32_t trinketMaxGoldDelta = 0;
    std::int32_t trinketImmediateGold = 0;
    //! Ornate Clock (BG32_MagicItem_271) moves the next Greater Trinket
    //! offer to the following recruit start.  This is player-owned rather
    //! than a global turn flag so duplicate/replayed players cannot leak it.
    bool ornateClockGreaterNextTurn = false;
    //! Mysterious Orb forces the next valid Trinket offer to be Lesser.
    //! This remains armed across a blocked/full modal boundary and is
    //! cleared only after the offer has been committed to pending state.
    bool mysteriousOrbLesserNext = false;
    //! Per-trigger counters for refresh/self-damage Trinket families.  These
    //! are intentionally player state rather than card-instance locals so a
    //! refresh cannot reset progress or make a replay depend on pointers.
    //! Number of end-of-recruit effects that increase the next turn's gold cap.
    std::int32_t trinketEndTurnMaxGold = 0;
    std::vector<Season14RaceShopStats> persistentShopRaceStats;
    std::vector<Season14RaceShopStats> persistentRaceStats;
    std::int32_t refreshRandomShopAttack = 0;
    std::int32_t refreshRandomShopHealth = 0;
    std::int32_t fodderRefreshes = 0;
    //! One-shot Fodders armed by Bloodfury Shield and similar effects.  This
    //! is deliberately separate from fodderRefreshes, whose semantics are a
    //! contribution on each of several future refreshes (Defiler/Demonology).
    std::int32_t foddersNextRefresh = 0;
    std::int32_t goldenMinionsPlayed = 0;
    std::int32_t piratesPlayedThisGame = 0;
    //! Lifetime Pirate acquisitions, including cards added to hand.  This is
    //! distinct from played/summoned counters so moving a card to the board
    //! cannot count the same acquisition twice.
    std::int32_t piratesAcquiredThisGame = 0;
    std::int32_t unboundElementals = 0;
    std::vector<std::string> combatAvengeCards;
    std::int32_t foddersPerRefresh = 1;
    //! Cumulative health added by future Tavern spells this game.
    std::int32_t tavernSpellHealthBonus = 0;
    //! Cumulative attack added by future Tavern spells this game.
    std::int32_t tavernSpellAttackBonus = 0;
    std::int32_t tavernLightingAttack = 1;
    std::int32_t tavernLightingHealth = 1;
    std::int32_t tavernLightingTurns = 0;
    std::int32_t deferredMinionAttack = 0;
    std::int32_t deferredMinionHealth = 0;
    std::uint8_t deferredMinionStatTurns = 0;
    std::int32_t persistentBeetleAttack = 0;
    std::int32_t persistentBeetleHealth = 0;
    std::int32_t nextTavernSpellDiscount = 0;
    std::int32_t lastTavernSpellDbfID = 0;
    //! Stable public entity targeted by the most recently resolved Tavern
    //! spell, when that target was in Bob's Tavern.  This is consumed by
    //! target-aware Trinket hooks before the next spell overwrites it.
    std::uint64_t lastTavernSpellShopTargetEntityID = 0;
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
    std::int32_t goldSpentThisGame = 0;
    std::int32_t heroDamageThisTurn = 0;
    //! Number of minions bought this recruit turn for Sharpen Blades.
    std::int32_t sharpenBladesPurchases = 0;
    //! Cloning Gallery is a once-per-game exact board-copy activation.
    bool cloningGalleryUsed = false;
    std::int32_t buriedTreasureDigs = 0;
    bool firstKillCopyArmed = false;
    std::optional<Minion> firstKillCopy;
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
    bool BeginTransformDecision(std::uint64_t sourceEntityID,
                                std::int32_t sourceCardDbfID,
                                std::uint64_t targetEntityID,
                                std::int32_t targetIndex,
                                std::int32_t targetTier);
    void CancelTransformDecision() noexcept;
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
    bool TakeVoidPowerDiscoverReady() noexcept
    {
        return ConsumeVoidPowerDiscover(heroPowerBatch6);
    }
    bool TakeFeelDevastationDiscoverReady() noexcept
    {
        return ConsumeFeelDevastationDiscover(heroPowerBatch6);
    }
    void RestoreFeelDevastationDiscoverReady() noexcept
    {
        RosettaStone::Battlegrounds::RestoreFeelDevastationDiscoverReady(
            heroPowerBatch6);
    }
    void RestoreVoidPowerDiscoverReady() noexcept
    {
        RosettaStone::Battlegrounds::RestoreVoidPowerDiscoverReady(
            heroPowerBatch6);
    }
    //! Consumes the scheduled Upbeat Harmony trigger, if this is turn 3n.
    bool TakeUpbeatHarmonyCopyReady() noexcept
    {
        return RosettaStone::Battlegrounds::TakeUpbeatHarmonyCopyReady(
            heroPowerBatch1);
    }

    //! Advances target-free Batch-4 lifecycle state at recruit start.
    void BeginRecruitTurnBatch4();
    void BeginRecruitTurnBatch5();
    void RecordRefreshBatch5();

    //! Resets per-combat Avenge progress before combat starts.
    void BeginCombatBatch4();

    //! Applies a successful minion sale to deferred hero-power state.
    void OnSellMinion();
    std::int32_t SoldMinionsThisTurn() const noexcept { return soldMinionsThisTurn; }

    //! Returns immediate bonus gold for a successfully purchased minion.
    std::int32_t OnBuyMinion(bool purchasedPirate);
    void OnBuyMinionSharpenBlades() noexcept;
    //! Returns +2/+1 per minion bought during this recruit turn.
    std::pair<std::int32_t, std::int32_t> SharpenBladesStats() const noexcept;
    bool CanBuriedTreasureDig() const noexcept;
    void RecordBuriedTreasureDig() noexcept;
    void ArmFirstKillCopy() noexcept;
    void RecordFirstKillCopy(const Minion& minion);
    bool TakeFirstKillCopy(Minion& out);
    void ExpireFirstKillCopy() noexcept;

    //! Returns the one-time attack bonus for the next minion purchase.
    Season14HeroPowerBatch4Result OnBuyMinionBatch4();

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
    //! Electrode Attractor sets Magnetic Mech purchases to exactly 2 Gold.
    //! This is an absolute cost aura, so multiple copies do not stack down.
    bool HasMagneticMechFixedCost() const noexcept
    { return magneticMechPurchaseCostDiscount > 0; }
    std::int32_t MagneticMechPurchaseCostDiscount() const noexcept
    { return magneticMechPurchaseCostDiscount; }
    void SetTemporaryMinionPurchaseCost(std::int32_t cost) noexcept
    {
        temporaryMinionPurchaseCost = cost < 0 ? -1 : cost;
    }

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

    //! Arms upcoming refresh fills with Tier-6-only offers.
    void ArmTierSixOnlyRefresh(std::int32_t count);

    //! Returns and clears the one-shot higher-tier refresh allowance.
    std::int32_t TakeHigherTierRefresh();

    //! Returns and clears the pending Tier-6-only fill allowance.
    std::int32_t TakeTierSixOnlyRefresh();

    //! Resolve refresh-triggered allowances before the Tavern is filled.
    void BeginRefreshTavern();

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
    void AddTemporaryTavernSpellStats(std::int32_t attack,
                                      std::int32_t health) noexcept
    { temporaryTavernSpellAttack += attack; temporaryTavernSpellHealth += health; }
    std::int32_t TakeGrowingSummonAttack(std::int32_t entityIndex,
                                          std::int32_t initialAttack,
                                          std::int32_t increment);
    void AddBloodGemRaceBonus(Race race, std::int32_t attack,
                              std::int32_t health);

    //! Records a successful Tavern refresh so one-shot effects are consumed.
    void OnRefreshTavern(bool refreshSucceeded);
    //! Resolve recruit-phase Trinket counters after authoritative events.
    //! Returns the permanent shop aura delta applied by this event.
    std::pair<std::int32_t, std::int32_t> OnRecruitHeroDamage(
        std::int32_t healthLost);
    //! Returns and clears the current refresh's temporary Tavern delta.
    std::pair<std::int32_t, std::int32_t> TakeRefreshShopStatsDelta() noexcept;
    //! Returns cumulative this-turn Tavern-spell stat bonuses.
    std::pair<std::int32_t, std::int32_t> TemporaryTavernSpellStats() const noexcept
    { return {temporaryTavernSpellAttack, temporaryTavernSpellHealth}; }
    //! Resolves Trinket Avenge progress after a friendly combat death.
    TrinketAvengeResult OnTrinketFriendlyMinionDied();
    void ResetTrinketAvengeProgress() noexcept;
    void OnFriendlyPirateAttack();
    void OnFriendlyMinionAttack();
    bool MaybeBeginNagaConquest(std::int32_t totalAttack) noexcept;
    std::int32_t AzsharaWarbandAttack() const noexcept { return azsharaWarbandAttack; }
    bool AzsharaConquestStarted() const noexcept { return azsharaConquestStarted; }
    bool ExpeditionTierPending(int tier) const noexcept
    { return tier == 2 || tier == 4 || tier == 6 ? (expeditionDiscoverMask & (1u << tier)) == 0 : false; }
    void MarkExpeditionTierDiscovered(int tier) noexcept
    { if (tier == 2 || tier == 4 || tier == 6) expeditionDiscoverMask |= static_cast<std::uint8_t>(1u << tier); }
    void SetExpeditionReward(int tier, std::int32_t dbfID) noexcept
    { if (tier == 2) expeditionRewardDbfIDs[0] = dbfID; else if (tier == 4) expeditionRewardDbfIDs[1] = dbfID; else if (tier == 6) expeditionRewardDbfIDs[2] = dbfID; }
    std::int32_t ExpeditionReward(int tier) const noexcept
    { return tier == 2 ? expeditionRewardDbfIDs[0] : tier == 4 ? expeditionRewardDbfIDs[1] : tier == 6 ? expeditionRewardDbfIDs[2] : 0; }
    void ClearExpeditionReward(int tier) noexcept
    { if (tier == 2) expeditionRewardDbfIDs[0] = 0; else if (tier == 4) expeditionRewardDbfIDs[1] = 0; else if (tier == 6) expeditionRewardDbfIDs[2] = 0; }
    bool SkipFirstRecruitTurn() const noexcept { return expeditionSkipFirstRecruitTurn; }
    void ConsumeFirstRecruitTurnSkip() noexcept { expeditionSkipFirstRecruitTurn = false; }
    std::uint8_t HeroicInspirationProgress() const noexcept
    { return heroicInspirationAttacks; }
    bool HeroicInspirationRewardPending() const noexcept
    { return heroicInspirationRewardPending; }
    void ClearHeroicInspirationReward() noexcept
    { heroicInspirationRewardPending = false; heroicInspirationAttacks = 0; }

    //! Records a successfully resolved Tavern spell.
    // `spellOnMinion` distinguishes targeted/board spell resolutions for
    // Trinkets whose improvement is only armed after a spell is cast on a
    // minion (for example Honeycomb Ring).  Keep the default for callers
    // that resolve non-targeted Tavern spells.
    void OnTavernSpellResolved(bool spellResolved, std::int32_t sourceDbfID = 0,
                               bool spellOnMinion = false,
                               std::uint64_t shopTargetEntityID = 0);
    std::int32_t SuccessfulSpellCount() const noexcept { return successfulSpellCount; }
    std::int32_t SuccessfulSpellsThisRecruitTurn() const noexcept
    {
        return std::max<std::int32_t>(0, successfulSpellCount -
                                         successfulSpellCountAtRecruitStart);
    }
    std::int32_t PendingSpellCountNagaRewards() const noexcept
    { return pendingSpellCountNagaRewards; }
    bool ConsumeSpellCountNagaReward() noexcept
    { if (pendingSpellCountNagaRewards <= 0) return false; --pendingSpellCountNagaRewards; return true; }
    std::int32_t TakeSpellCountGold() noexcept
    { const auto n = pendingSpellCountGold; pendingSpellCountGold = 0; return n; }
    std::int32_t TakeSpellCountBloodGems() noexcept
    { const auto n = pendingSpellCountBloodGems; pendingSpellCountBloodGems = 0; return n; }
    std::int32_t TakeSpellMinionAttackDelta() noexcept
    { const auto d = spellMinionAttackDelta; spellMinionAttackDelta = 0; return d; }
    std::int32_t LastTavernSpellDbfID() const noexcept { return lastTavernSpellDbfID; }

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
    void ArmCombatHandSummon(Minion snapshot, std::int32_t count)
    {
        if (count > 0)
            pendingCombatHandSummons.emplace_back(std::move(snapshot), count);
    }
    std::int32_t ResolveTierSixTrinketGold() noexcept;
    std::int32_t ResolveDelayedTrinketGold() noexcept;
    std::pair<std::int32_t, std::int32_t> TakeSpellCastMinionStats() noexcept
    { auto r = std::pair{spellCastMinionAttackDelta, spellCastMinionHealthDelta}; spellCastMinionAttackDelta = spellCastMinionHealthDelta = 0; return r; }
    std::vector<std::pair<Minion, std::int32_t>> TakeCombatHandSummons()
    {
        auto out = std::move(pendingCombatHandSummons);
        pendingCombatHandSummons.clear();
        return out;
    }
    void ArmBloodGemGolemAttack(std::uint64_t targetEntityID,
                                std::int32_t attack, std::int32_t health)
    { pendingBloodGemGolemAttacks.push_back({targetEntityID, attack, health}); }
    std::vector<PendingBloodGemGolemAttack> TakeBloodGemGolemAttacks()
    { auto out = std::move(pendingBloodGemGolemAttacks); pendingBloodGemGolemAttacks.clear(); return out; }
    std::size_t ArmExactCopyDeathrattle(Minion snapshot)
    {
        pendingExactCopySnapshots.emplace_back(std::move(snapshot));
        return pendingExactCopySnapshots.size() - 1;
    }
    std::optional<Minion> TakeExactCopyDeathrattle(std::size_t id)
    {
        if (id >= pendingExactCopySnapshots.size() ||
            !pendingExactCopySnapshots[id].has_value())
            return std::nullopt;
        auto result = std::move(pendingExactCopySnapshots[id]);
        pendingExactCopySnapshots[id].reset();
        return result;
    }
    void ClearCombatExactCopySnapshots() noexcept
    {
        pendingExactCopySnapshots.clear();
    }
    void ArmCombatStartLeftmostAttackDouble(std::int32_t sourceCardDbfID) noexcept;
    std::size_t TakeCombatStartLeftmostAttackDoubles() noexcept;
    void ArmCombatStartNearestStats(std::int32_t sourceCardDbfID) noexcept;
    void ArmShopBloodGemsOnRefresh(std::int32_t sourceCardDbfID) noexcept;
    bool HasShopBloodGemsOnRefresh() const noexcept;
    void ArmRefreshRace(Race race) noexcept { pendingRefreshRace = race; }
    Race TakeRefreshRace() noexcept { const auto race = pendingRefreshRace; pendingRefreshRace = Race::INVALID; return race; }
    void ArmCombatStartBeetles(std::int32_t sourceCardDbfID) noexcept;
    std::size_t TakeCombatStartBeetles() noexcept;
    std::size_t TakeCombatStartNearestStats() noexcept;
    void ArmCombatStartRandomEnemySetHealth(std::int32_t sourceCardDbfID) noexcept;
    std::size_t TakeCombatStartRandomEnemySetHealth() noexcept;
    void RecordMinionPlay(bool battlecry) noexcept
    {
        ++minionsPlayedThisTurn;
        if (battlecry) ++battlecriesTriggered;
    }
    bool WasMinionPlayedThisTurn(std::string_view cardID) const noexcept
    {
        return std::find(repeatedPlayCardIDs.begin(), repeatedPlayCardIDs.end(),
                         cardID) != repeatedPlayCardIDs.end();
    }
    void RecordRepeatedPlayCard(std::string_view cardID)
    {
        if (!WasMinionPlayedThisTurn(cardID))
            repeatedPlayCardIDs.emplace_back(cardID);
    }
    void RecordBattlecry() noexcept { ++battlecriesTriggered; }
    int RecordGoldSpent(std::int32_t amount) noexcept;
    void SetChampionReward(std::int32_t dbfID) noexcept { championRewardDbfID = dbfID; }
    bool ChampionRewardPending() const noexcept { return championRewardDbfID != 0; }
    std::int32_t GoldSpentThisGame() const noexcept { return goldSpentThisGame; }

    //! Returns and clears deferred next-turn gold.
    std::int32_t TakeNextTurnGold() noexcept;

    //! Returns and clears gold granted when a Trinket was selected.
    std::int32_t TakeImmediateGold() noexcept;
    //! Returns and clears max-Gold increases armed for the current recruit end.
    std::int32_t TakeEndTurnMaxGold() noexcept;

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
    //! Improves each subsequently resolved Deep Blues Spellcraft card.
    void ImproveFutureDeepBlues(std::int32_t attack,
                                std::int32_t health) noexcept;
    std::pair<std::int32_t, std::int32_t> FutureDeepBluesStats() const noexcept;
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
    //! Arms one Fodder per count for the next successful refresh only.
    //! Multiple copies stack, but there is no useful state beyond Tavern
    //! capacity; cap the pending queue so repeated casts cannot leak into
    //! later refreshes after the next refresh has resolved.
    void ArmFoddersNextRefresh(std::int32_t count = 1) noexcept
    {
        if (count > 0)
            foddersNextRefresh = std::min(
                MAX_FIELD_SIZE, foddersNextRefresh + count);
    }
    std::int32_t ConsumeFoddersNextRefresh() noexcept
    {
        const auto result = foddersNextRefresh;
        foddersNextRefresh = 0;
        return result;
    }
    //! Arms the shared next-three-refresh window used by Defiler triggers.
    //! Multiple Defilers resolve together, so their window is not extended.
    void ArmFodderDefilerRefreshes(std::int32_t count,
                                   std::int32_t amount = 1) noexcept
    {
        if (count > 0) {
            // The window is shared, but each Defiler contributes its own
            // Fodder to every refresh in that window.  Preserve the longest
            // remaining window while summing simultaneous contributions.
            fodderRefreshes = std::max(fodderRefreshes, count);
            foddersPerRefresh += amount;
        }
    }
    std::int32_t ConsumeFodderRefresh() noexcept { if (fodderRefreshes <= 0) return 0; --fodderRefreshes; const auto result = foddersPerRefresh; if (fodderRefreshes == 0) foddersPerRefresh = 1; return result; }
    void TrackCombatAvengeCard(std::string id) { combatAvengeCards.push_back(std::move(id)); }
    std::vector<std::string> TakeCombatAvengeCards() { auto result = std::move(combatAvengeCards); combatAvengeCards.clear(); return result; }

    //! Resolves a complete target-free Batch-3 activation for this hero.
    bool ResolveHeroPowerBatch3Activation(
        std::int32_t currentTier,
        Season14HeroPowerBatch3Activation& result) const noexcept;
    void ArmCthunEndTurnTargets(const std::array<std::uint64_t, 32>& ids,
                                std::uint8_t count, std::uint8_t applications) noexcept;
    void ArmTierMinionStartCombat(std::int32_t tier) noexcept {
        tierMinionStartCombatPending = true;
        tierMinionStartCombatTier = tier;
    }
    bool TakeTierMinionStartCombat(std::int32_t& tier) noexcept {
        if (!tierMinionStartCombatPending) return false;
        tier = tierMinionStartCombatTier;
        tierMinionStartCombatPending = false;
        tierMinionStartCombatTier = 0;
        return true;
    }
    bool HasCthunEndTurnTargets() const noexcept { return cthunEndTurnPending; }

    //! Returns a passive combat-kill bonus owned by this hero, if any.
    std::int32_t HeroPowerBatch3CombatKillAttackBonus() const noexcept;
    //! Record one confirmed friendly-owned enemy kill. Returns true exactly
    //! when Sulfuras reaches its once-per-combat threshold.
    bool RecordFriendlyCombatKill() noexcept;

    //! Returns whether the player can pay and use the power this turn.
    bool CanUseHeroPower(std::int32_t availableGold) const;

    //! Returns the current cost after deterministic hero-power discounts.
    std::int32_t EffectiveHeroPowerCost() const;
    void RecordLastOpponentCombatMinions(
        const std::vector<std::int32_t>& dbfIDs);
    void RecordLastOpponentCombatMinionSnapshots(
        const std::vector<Minion>& minions);
    void RecordLastOpponentBuddy(std::int32_t dbfID) noexcept
    {
        lastOpponentBuddyDbfID = std::max<std::int32_t>(0, dbfID);
    }
    std::optional<Minion> LastOpponentCombatMinionSnapshot(
        std::size_t index) const;
    std::optional<Minion> FindLastOpponentCombatMinionSnapshot(
        std::int32_t dbfID) const;
    bool ArmRapidReanimation(std::uint64_t entityID, Minion snapshot);
    bool TakeRapidReanimationSnapshot(Minion& out) noexcept;

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
