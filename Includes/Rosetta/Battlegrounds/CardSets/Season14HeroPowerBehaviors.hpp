// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! The executable behavior family for a pinned Season 14 hero power.
//!
//! This is intentionally a small, explicit registry.  It is not inferred
//! from card text: an ID may enter the supported pool only after its family
//! has a corresponding simulator hook and tests.
enum class Season14HeroPowerKind : std::uint8_t
{
    STARTING_HEALTH,
    TAVERN_MINION_AURA,
    ECONOMY_COST_AURA,
    GOLD_SCALING,
    FREE_REFRESH,
    BUY_PIRATE_GOLD,
    TAVERN_SPELL_AURA,
    MAX_GOLD,
    RANDOM_TAVERN_SPELL,
    START_TURN_SPELLCRAFT,
    MECHGYVER,
    BOON_OF_LIGHT,
    SHARPEN_BLADES,
    BURIED_TREASURE,
    FIRST_KILL_COPY,
    SEE_THE_LIGHT,
    BRICK_BY_BRICK,
    GONNA_BE_RICH,
    LEAD_EXPLORER,
    CLONING_GALLERY,
    KING_OF_DUALITY,
    UPBEAT_HARMONY,
    NATURAL_BALANCE,
    SPIRIT_SWAP,
    GALAKROND_GREED,
    DUNGARS_GRYPHON,
    DEVOUR,
    I_SPY,
    POWER_OF_THE_STORM,
    LUCKY_ROLL,
    NAGA_CONQUEST,
    REBORN_RITES,
    SNICKER_SNACK,
    STIR_THE_POT,
    RECLAIMED_SOULS,
    QUEEN_OF_DRAGONS,
    IMPRISON,
    SIGN_NEW_ARTIST,
    CONVICTION,
    PERFECT_CRIME,
    DETECTIVE_FOR_HIRE,
    RAPID_REANIMATION,
    FANTASTIC_TREASURE,
    LIFT_OFF,
    WARP_GATE,
};

struct Season14HeroPowerDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerKind kind;
    std::int32_t cost;
    bool passive;
    // Optional linked Buddy DBF ID from the pinned hero metadata.
    std::int32_t buddyDbfID = 0;
};

//! Generated quest-reward cards are modal options rather than ordinary
//! minions/spells. Keep their pinned IDs in one typed pool so replayed choice
//! payloads can be validated without trusting card text or UI labels.
struct Season14GeneratedChoiceDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    //! The reward's typed rules contract.  The executable bit is set only
    //! after a matching Player lifecycle hook and focused test exist.
    enum class Effect : std::uint8_t
    {
        END_TURN_BATTLECRY,
        START_COMBAT_GOLDEN_FLANKS,
        START_COMBAT_HIGHEST_HEALTH_COPY,
        DEATHRATTLE_DEATH_BUFF,
        END_TURN_RIGHTMOST_STEALTH_HEALTH,
        FIRST_BATTLECRY_REPEAT,
        GLOBAL_ATTACK_AURA,
        REFRESH_RANDOM_MINION_BUFF,
        DISCOVER_COPY,
        START_TURN_HAND_BUFF,
        END_TURN_EXTRA_TRIGGER,
        START_TURN_RANDOM_CARDS,
    } effect;
    bool executable = false;
    std::string_view missingLinkReason = {};
};

inline constexpr std::array<Season14GeneratedChoiceDefinition, 12>
    SEASON14_GENERATED_QUEST_REWARDS = {{
        {"BG24_Reward_107", 89449, Season14GeneratedChoiceDefinition::Effect::END_TURN_BATTLECRY, true},
        {"BG24_Reward_109", 89473, Season14GeneratedChoiceDefinition::Effect::START_COMBAT_GOLDEN_FLANKS, true},
        {"BG24_Reward_111", 89481, Season14GeneratedChoiceDefinition::Effect::START_COMBAT_HIGHEST_HEALTH_COPY, true},
        {"BG24_Reward_113", 89483, Season14GeneratedChoiceDefinition::Effect::DEATHRATTLE_DEATH_BUFF, true},
        {"BG24_Reward_115", 89947, Season14GeneratedChoiceDefinition::Effect::END_TURN_RIGHTMOST_STEALTH_HEALTH, true},
        {"BG24_Reward_123", 90861, Season14GeneratedChoiceDefinition::Effect::FIRST_BATTLECRY_REPEAT, true},
        {"BG24_Reward_125", 90865, Season14GeneratedChoiceDefinition::Effect::GLOBAL_ATTACK_AURA, true},
        {"BG24_Reward_128", 90914, Season14GeneratedChoiceDefinition::Effect::REFRESH_RANDOM_MINION_BUFF, true},
        // These generated quest-reward options are pinned from cards.json.
        // Entries without complete linked-entity data remain metadata-only
        // until their event/state contracts are implemented.
        {"BG24_Reward_129", 90916, Season14GeneratedChoiceDefinition::Effect::DISCOVER_COPY, true},
        {"BG24_Reward_130", 90917, Season14GeneratedChoiceDefinition::Effect::END_TURN_EXTRA_TRIGGER, false, "placeholder_copy_target_0"},
        {"BG24_Reward_131", 92542, Season14GeneratedChoiceDefinition::Effect::START_TURN_HAND_BUFF, true},
        {"BG24_Reward_134", 92551, Season14GeneratedChoiceDefinition::Effect::START_TURN_RANDOM_CARDS, false, "placeholder_random_card_92"},
    }};

constexpr bool IsSeason14GeneratedQuestReward(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_GENERATED_QUEST_REWARDS)
        if (definition.dbfID == dbfID) return true;
    return false;
}

constexpr const Season14GeneratedChoiceDefinition*
FindSeason14GeneratedQuestReward(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_GENERATED_QUEST_REWARDS)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

//! Only choices with a complete Player lifecycle may earn executable
//! behavior coverage; metadata-only options remain fail-closed.
constexpr bool IsExecutableSeason14GeneratedQuestReward(
    std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14GeneratedQuestReward(dbfID);
    return definition != nullptr && definition->executable;
}

constexpr std::string_view GeneratedQuestRewardMissingLinkReason(
    std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14GeneratedQuestReward(dbfID);
    return definition == nullptr ? std::string_view{"unknown_reward"}
                                 : definition->missingLinkReason;
}

//! Exact Patch 36.4 behavior batch (eight distinct reusable families).
inline constexpr std::array<Season14HeroPowerDefinition, 45>
    SEASON14_HERO_POWER_BEHAVIORS = {{
        {"TB_BaconShop_HP_035", 59399,
         Season14HeroPowerKind::STARTING_HEALTH, 0, true},
        {"BG20_HERO_102p", 71455,
         Season14HeroPowerKind::TAVERN_MINION_AURA, 0, true},
        {"TB_BaconShop_HP_054", 60405,
         Season14HeroPowerKind::ECONOMY_COST_AURA, 0, true},
        {"TB_BaconShop_HP_076", 62269,
         Season14HeroPowerKind::GOLD_SCALING, 0, false},
        {"TB_BaconShop_HP_063", 61491,
         Season14HeroPowerKind::FREE_REFRESH, 0, true},
        {"BG26_HERO_101p", 101132,
         Season14HeroPowerKind::BUY_PIRATE_GOLD, 0, true},
        {"TB_BaconShop_HP_085t", 122960,
         Season14HeroPowerKind::TAVERN_SPELL_AURA, 0, true},
        {"BG32_HERO_001p", 116921,
         Season14HeroPowerKind::MAX_GOLD, 3, false},
        {"BG28_HERO_801p", 110472,
         Season14HeroPowerKind::RANDOM_TAVERN_SPELL, 1, false},
        {"BG23_HERO_304p", 85126,
         Season14HeroPowerKind::START_TURN_SPELLCRAFT, 1, true},
        {"BG22_HERO_200p", 81572,
         Season14HeroPowerKind::MECHGYVER, 9, true},
        {"BG21_HERO_000p", 73941,
         Season14HeroPowerKind::CONVICTION, 0, false},
        {"TB_BaconShop_HP_010", 57562,
         Season14HeroPowerKind::BOON_OF_LIGHT, 2, false},
        {"TB_BaconShop_HP_001", 57567,
         Season14HeroPowerKind::SHARPEN_BLADES, 1, false},
        {"TB_BaconShop_HP_074", 62250,
         Season14HeroPowerKind::BURIED_TREASURE, 1, false},
        {"TB_BaconShop_HP_053", 60381,
         Season14HeroPowerKind::FIRST_KILL_COPY, 1, false},
        {"BG20_HERO_101p", 70957,
         Season14HeroPowerKind::SEE_THE_LIGHT, 2, false},
        {"TB_BaconShop_HP_040", 59832,
         Season14HeroPowerKind::BRICK_BY_BRICK, 0, false},
        {"TB_BaconShop_HP_046", 60216,
         Season14HeroPowerKind::GONNA_BE_RICH, 0, false},
        {"TB_BaconShop_HP_047", 60217,
         Season14HeroPowerKind::LEAD_EXPLORER, 1, false},
        {"BG31_HERO_005p", 117410,
         Season14HeroPowerKind::CLONING_GALLERY, 0, false},
        {"BG35_HERO_001p", 129685,
         Season14HeroPowerKind::KING_OF_DUALITY, 0, false},
        {"BG26_HERO_104p", 99034,
         Season14HeroPowerKind::UPBEAT_HARMONY, 0, false},
        {"BG20_HERO_242p", 68130,
         Season14HeroPowerKind::NATURAL_BALANCE, 2, false},
        {"BG20_HERO_201p", 71464,
         Season14HeroPowerKind::SPIRIT_SWAP, 0, false},
        {"TB_BaconShop_HP_011", 57555,
         Season14HeroPowerKind::GALAKROND_GREED, 1, false},
        {"BG20_HERO_283p", 75703,
         Season14HeroPowerKind::DUNGARS_GRYPHON, 0, false},
        {"BG20_HERO_301p", 71914,
         Season14HeroPowerKind::DEVOUR, 0, false},
        {"BG21_HERO_010p", 76563,
         Season14HeroPowerKind::I_SPY, 2, false},
        {"BG20_HERO_202p", 71909,
         Season14HeroPowerKind::POWER_OF_THE_STORM, 0, false},
        {"BG28_HERO_400p", 105315,
         Season14HeroPowerKind::LUCKY_ROLL, 1, false},
        {"BG22_HERO_007p2", 80007,
         Season14HeroPowerKind::NAGA_CONQUEST, 1, false},
        {"TB_BaconShop_HP_024", 58040,
         Season14HeroPowerKind::REBORN_RITES, 0, false},
        {"TB_BaconShop_HP_022", 58028,
         Season14HeroPowerKind::SNICKER_SNACK, 0, false},
        {"BG21_HERO_020p", 77434,
         Season14HeroPowerKind::STIR_THE_POT, 0, false},
        {"BG23_HERO_306p", 89294,
         Season14HeroPowerKind::RECLAIMED_SOULS, 2, false},
        {"TB_BaconShop_HP_064", 61517,
         Season14HeroPowerKind::QUEEN_OF_DRAGONS, 1, false},
        {"TB_BaconShop_HP_068", 61919,
         Season14HeroPowerKind::IMPRISON, 1, false},
        {"BG25_HERO_105p", 101346,
         Season14HeroPowerKind::SIGN_NEW_ARTIST, 3, false, 101349},
        // The metadata text says 10, but the Patch 36.4 rules contract uses
        // the 11-cost starting value; one point is discounted at each
        // recruit boundary before the next activation.
        {"BG23_HERO_305p", 86292,
         Season14HeroPowerKind::PERFECT_CRIME, 11, false},
        {"BG23_HERO_303p2", 90403,
         Season14HeroPowerKind::DETECTIVE_FOR_HIRE, 0, false},
        {"BG25_HERO_103p", 98728,
         Season14HeroPowerKind::RAPID_REANIMATION, 3, false},
        {"BG30_HERO_304p", 113311,
         Season14HeroPowerKind::FANTASTIC_TREASURE, 0, false},
        {"BG31_HERO_801p", 118681,
         Season14HeroPowerKind::LIFT_OFF, 0, true},
        {"BG31_HERO_802p", 119196,
         Season14HeroPowerKind::WARP_GATE, 0, true},
    }};

constexpr const Season14HeroPowerDefinition* FindSeason14HeroPowerBehavior(
    std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerDefinition* FindSeason14HeroPowerBehavior(
    std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr bool HasSeason14HeroPowerBehavior(std::int32_t dbfID) noexcept
{
    return FindSeason14HeroPowerBehavior(dbfID) != nullptr;
}

//! Declarative target contract for the executable See the Light family.
//! Keeping this beside the pinned registry prevents the bridge from growing
//! another ID-only special case when the same lifecycle is reused by a
//! generated hero-power variant.
constexpr bool Season14HeroPowerUsesTavernTarget(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::SEE_THE_LIGHT;
}

constexpr bool Season14HeroPowerTavernTargetIsLegal(
    std::int32_t dbfID, bool handHasRoom, std::int32_t tavernMinionCount) noexcept
{
    return Season14HeroPowerUsesTavernTarget(dbfID) && handHasRoom &&
           tavernMinionCount > 0;
}

//! Returns whether a hero power applies its effect to one minion per tier.
constexpr bool Season14HeroPowerUsesNaturalBalance(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::NATURAL_BALANCE;
}

//! Returns whether a hero power consumes two distinct friendly board targets.
constexpr bool Season14HeroPowerUsesSpiritSwap(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::SPIRIT_SWAP;
}

constexpr bool Season14HeroPowerUsesGalakrondGreed(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::GALAKROND_GREED;
}

constexpr bool Season14HeroPowerUsesNagaConquest(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::NAGA_CONQUEST;
}

constexpr bool Season14HeroPowerUsesDevour(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::DEVOUR;
}

constexpr bool Season14HeroPowerUsesISpy(std::int32_t dbfID) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    return definition != nullptr &&
           definition->kind == Season14HeroPowerKind::I_SPY;
}

//! Deterministic player-owned modifiers for the supported passive families.
//!
//! The registry also contains target-dependent/random powers.  Those powers
//! intentionally leave these fields at their neutral values until their
//! target/random contract is implemented by the simulator.
struct Season14HeroPowerBatch1State
{
    //! Recruit turns are one-based and scoped to the selected hero power.
    std::int32_t turnNumber = 0;
    //! Upbeat Harmony arms a plain left-most hand copy every third turn.
    bool upbeatHarmonyCopyReady = false;
    //! Extra starting health applied to the hero's imported base health.
    //! Patch 36.4 describes All Patched Up as +30 Health, not an absolute
    //! floor; preserving this as a delta also handles future base-health
    //! variants without silently under- or over-applying the effect.
    std::int32_t startingHealthBonus = 0;
    std::int32_t minionCostDelta = 0;
    std::int32_t refreshCostDelta = 0;
    std::int32_t upgradeCostDelta = 0;
    std::int32_t tavernSpellCostDelta = 0;
    bool freeRefreshAvailable = false;
    //! Piggy Bank is a once-per-game activation; unlike ordinary hero powers
    //! its use must not be reopened by the next recruit turn.
    bool piggyBankUsed = false;
    std::int32_t brickByBrickHealth = 2;
    std::int32_t brickByBrickTurn = 0;
    bool brickByBrickUsedThisTurn = false;
    bool gonnaBeRichUsed = false;
    std::int32_t leadExplorerCostDelta = 0;
    bool kingOfDualityOffered = false;

    constexpr std::int32_t MinionCost(std::int32_t baseCost) const noexcept
    {
        return baseCost + minionCostDelta < 0 ? 0
                                               : baseCost + minionCostDelta;
    }

    constexpr std::int32_t StartingHealth(
        std::int32_t metadataHealth) const noexcept
    {
        return metadataHealth + startingHealthBonus;
    }

    constexpr std::int32_t RefreshCost(std::int32_t baseCost) const noexcept
    {
        return baseCost + refreshCostDelta < 0
                   ? 0
                   : baseCost + refreshCostDelta;
    }

    constexpr std::int32_t UpgradeCost(std::int32_t baseCost) const noexcept
    {
        return baseCost + upgradeCostDelta < 0
                   ? 0
                   : baseCost + upgradeCostDelta;
    }

    constexpr std::int32_t TavernSpellCost(
        std::int32_t baseCost) const noexcept
    {
        return baseCost + tavernSpellCostDelta < 0
                   ? 0
                   : baseCost + tavernSpellCostDelta;
    }

    constexpr bool ConsumeFreeRefresh() noexcept
    {
        if (!freeRefreshAvailable)
        {
            return false;
        }
        freeRefreshAvailable = false;
        return true;
    }
};

//! Return the deterministic passive modifiers for a selected hero power.
constexpr Season14HeroPowerBatch1State
Season14HeroPowerBatch1Modifiers(std::int32_t dbfID) noexcept
{
    switch (dbfID)
    {
        case 59399: // Patchwerk: All Patched Up.
            return {.startingHealthBonus = 30};
        case 60405: // Millhouse Manastorm: Manastorm.
            return {.minionCostDelta = 2,
                    .refreshCostDelta = 2,
                    .upgradeCostDelta = 1};
        case 122960: // Rakanishu: Tavern Lighting.
            // Tavern Lighting buffs the stats granted by Tavern spells. It
            // does not make those spells cheaper; its stat aura remains
            // pending the spell-effect executor and must not alter payment.
            return {};
        default:
            return {};
    }
}

//! Events owned by a player's deterministic Batch-1 lifecycle.
enum class Season14HeroPowerBatch1Event : std::uint8_t
{
    BEGIN_TURN,
    REFRESH_TAVERN,
};

//! Resolve the deterministic purchase hook used by Cap'n Hoggarr.
//! Targeted/random hero powers never use this helper and remain fail-closed.
constexpr std::int32_t Season14HeroPowerBatch1PurchaseGold(
    std::int32_t dbfID, bool purchasedPirate) noexcept
{
    return dbfID == 101132 && purchasedPirate ? 1 : 0;
}

//! Resolve the deterministic start/refresh hooks.  A free refresh is
//! consumed only when the caller reports that a refresh actually happened.
constexpr bool ResolveSeason14HeroPowerBatch1Event(
    std::int32_t dbfID, Season14HeroPowerBatch1Event event,
    Season14HeroPowerBatch1State& state, bool refreshSucceeded = false) noexcept
{
    if (event == Season14HeroPowerBatch1Event::BEGIN_TURN)
    {
        if (dbfID == 59832)
        {
            if (state.brickByBrickTurn > 0 &&
                !state.brickByBrickUsedThisTurn)
                ++state.brickByBrickHealth;
            ++state.brickByBrickTurn;
            state.brickByBrickUsedThisTurn = false;
        }
        if (dbfID == 61491) // Nozdormu: Clairvoyance.
        {
            state.freeRefreshAvailable = true;
        }
        return true;
    }
    if (event == Season14HeroPowerBatch1Event::REFRESH_TAVERN &&
        refreshSucceeded)
    {
        return state.ConsumeFreeRefresh();
    }
    return false;
}

//! Data-only effects produced by a no-target activation.
struct Season14HeroPowerActivation
{
    std::int32_t goldDelta = 0;
    std::int32_t maxGoldDelta = 0;
    std::int32_t healthDelta = 0;
    bool makeGolden = false;
    bool beginDiscover = false;
    bool consumesTurnUse = true;
};

//! Resolve the three-turn lifecycle for Rock Master Voone's Upbeat Harmony.
//! The copy itself is deliberately performed by Player, where hand card
//! reconstruction and capacity are authoritative; this state only schedules
//! the effect at the correct recruit boundary.
constexpr bool ResolveUpbeatHarmonyBeginTurn(
    std::int32_t dbfID, Season14HeroPowerBatch1State& state) noexcept
{
    const auto* definition = FindSeason14HeroPowerBehavior(dbfID);
    if (definition == nullptr ||
        definition->kind != Season14HeroPowerKind::UPBEAT_HARMONY)
        return false;
    ++state.turnNumber;
    state.upbeatHarmonyCopyReady = state.turnNumber % 3 == 0;
    return state.upbeatHarmonyCopyReady;
}

constexpr bool TakeUpbeatHarmonyCopyReady(
    Season14HeroPowerBatch1State& state) noexcept
{
    if (!state.upbeatHarmonyCopyReady) return false;
    state.upbeatHarmonyCopyReady = false;
    return true;
}

//! Resolve the two no-target active powers in this batch.
//!
//! `turnNumber` is one-based: the first recruit turn returns +1 for Piggy
//! Bank, the second returns +2, and so on.  Passive and target-dependent
//! powers return false so callers cannot accidentally claim an incomplete
//! implementation.
constexpr bool ResolveSeason14HeroPowerActivation(
    std::int32_t dbfID, std::int32_t turnNumber,
    Season14HeroPowerBatch1State& state,
    Season14HeroPowerActivation& result) noexcept
{
    result = {};
    if (dbfID == 62269) // TB_BaconShop_HP_076, Piggy Bank
    {
        if (state.piggyBankUsed)
        {
            return false;
        }
        // The printed +1 is the first-turn amount. A skipped/legacy zero
        // turn still receives the minimum printed amount, never zero.
        result.goldDelta = turnNumber > 0 ? turnNumber : 1;
        state.piggyBankUsed = true;
        return true;
    }
    if (dbfID == 116921) // BG32_HERO_001p, Wisdom of Ancients
    {
        result.maxGoldDelta = 1;
        return true;
    }
    if (dbfID == 59832) // Brick by Brick, +2 Health; grows when unused.
    {
        if (state.brickByBrickUsedThisTurn)
            return false;
        result.healthDelta = state.brickByBrickHealth;
        state.brickByBrickUsedThisTurn = true;
        return true;
    }
    if (dbfID == 60216) // Gonna Be Rich!, once per game.
    {
        if (state.gonnaBeRichUsed)
            return false;
        state.gonnaBeRichUsed = true;
        result.makeGolden = true;
        return true;
    }
    if (dbfID == 60217) // Lead Explorer, cost rises after each use.
    {
        result.beginDiscover = true;
        ++state.leadExplorerCostDelta;
        return true;
    }
    if (dbfID == 129685 && turnNumber >= 4 &&
        !state.kingOfDualityOffered)
    {
        state.kingOfDualityOffered = true;
        result.beginDiscover = true;
        return true;
    }
    return false;
}

//! Compatibility resolver for callers that only need the pure activation
//! payload. Player/bridge callers must use the stateful overload above so
//! once-per-game effects cannot be replayed after a turn reset.
constexpr bool ResolveSeason14HeroPowerActivation(
    std::int32_t dbfID, std::int32_t turnNumber,
    Season14HeroPowerActivation& result) noexcept
{
    Season14HeroPowerBatch1State state{};
    return ResolveSeason14HeroPowerActivation(dbfID, turnNumber, state, result);
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
