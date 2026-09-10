// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP

#include <array>
#include <cstddef>
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
    SPAWNING_POOL,
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

// Warp Gate is a passive lifecycle, not a repeatable hero-power activation.
// Keep its public contract typed so selection, lifetime purchase progress,
// and the pending two-choice reward cannot drift apart or silently become an
// active action.
struct WarpGateLifecycleDefinition
{
    std::int32_t heroPowerDbfID;
    std::int32_t buyThreshold;
    std::size_t choiceCount;
};

inline constexpr WarpGateLifecycleDefinition WARP_GATE_LIFECYCLE = {
    119196, 14, 2};

constexpr bool IsWarpGateHeroPowerDbfID(std::int32_t dbfID) noexcept
{
    return dbfID == WARP_GATE_LIFECYCLE.heroPowerDbfID;
}

// Protoss rewards are hero-generated and therefore are not required to be in
// the ordinary Tavern pool. Keep the pinned DBF boundary explicit: a missing
// generated behavior must fail closed instead of substituting another race.
inline constexpr std::array<std::int32_t, 10> WARP_GATE_PROTOSS_DBF_IDS = {
    // Zealot, Dark Templar, Carrier, and the generated Carrier Interceptor
    // are all members of the pinned Warp Gate reward pool.  Keep the linked
    // Interceptor here as well as in the combat spawn path: Mothership and
    // Warp Gate use this same allowlist for hand rewards.
    113732, 113165, 113174, 113175, 113177, 113203,
    113735, 113738, 113739, 113733};

constexpr bool IsWarpGateProtossDbfID(std::int32_t dbfID) noexcept
{
    for (const auto id : WARP_GATE_PROTOSS_DBF_IDS)
        if (id == dbfID) return true;
    return false;
}

constexpr bool IsExecutableWarpGateProtossDbfID(std::int32_t dbfID) noexcept
{
    return IsWarpGateProtossDbfID(dbfID);
}

// Whodunit's public start-game pool is the authoritative Season 14 quest
// token set. BG24_Quest_Bob is the generic quest-system prompt and must not be
// offered as a selectable quest itself.
inline constexpr std::array<std::int32_t, 17> WHODUNIT_QUEST_DBF_IDS = {
    89641, 89643, 89958, 92548, 92549, 92550, 95940, 90328,
    96152, 89011, 89037, 89044, 89052, 89057, 96153, 97674, 97743};

constexpr bool IsWhodunitQuestDbfID(std::int32_t dbfID) noexcept
{
    for (const auto id : WHODUNIT_QUEST_DBF_IDS)
        if (id == dbfID) return true;
    return false;
}

inline constexpr std::array<std::int32_t, 9> SPAWNING_POOL_ZERG_DBF_IDS = {
    120365, 120367, 120370, 120373, 120375,
    120378, 120381, 120383, 120386};
inline constexpr std::uint8_t SPAWNING_POOL_UNLOCK_TIER = 2;

constexpr bool IsSpawningPoolZergDbfID(std::int32_t dbfID) noexcept
{
    for (const auto id : SPAWNING_POOL_ZERG_DBF_IDS)
        if (id == dbfID) return true;
    return false;
}

enum class SpawningPoolZergEffect : std::uint8_t {
    SUMMON_COPY,
    END_TURN_TIER_HEALTH,
    RALLY_TIER_ATTACK,
    DEATHRATTLE_ATTACK_DAMAGE,
    KILL_ATTACK,
    AVENGE_STATS,
    ATTACKING_VENOMOUS,
    PLAY_CARD_BUFF,
    START_COMBAT_DOUBLE_STATS,
};

struct SpawningPoolZergBehaviorSpec {
    std::int32_t dbfID;
    std::int32_t goldenDbfID;
    std::uint8_t tier;
    SpawningPoolZergEffect effect;
    bool executable;
};

inline constexpr std::array<SpawningPoolZergBehaviorSpec, 9>
    SPAWNING_POOL_ZERG_BEHAVIORS = {{
        {120365, 120366, 1, SpawningPoolZergEffect::SUMMON_COPY, true},
        {120367, 120369, 1, SpawningPoolZergEffect::END_TURN_TIER_HEALTH, true},
        {120370, 120372, 1, SpawningPoolZergEffect::RALLY_TIER_ATTACK, true},
        {120373, 120374, 2, SpawningPoolZergEffect::DEATHRATTLE_ATTACK_DAMAGE, true},
        {120375, 120377, 2, SpawningPoolZergEffect::KILL_ATTACK, true},
        {120378, 120380, 2, SpawningPoolZergEffect::AVENGE_STATS, true},
        {120381, 120382, 3, SpawningPoolZergEffect::ATTACKING_VENOMOUS, true},
        {120383, 120385, 3, SpawningPoolZergEffect::PLAY_CARD_BUFF, true},
        {120386, 120388, 3, SpawningPoolZergEffect::START_COMBAT_DOUBLE_STATS, true},
    }};

constexpr bool IsExecutableSpawningPoolZergDbfID(std::int32_t dbfID) noexcept
{
    for (const auto& spec : SPAWNING_POOL_ZERG_BEHAVIORS)
        if (spec.dbfID == dbfID) return spec.executable;
    return false;
}

constexpr std::uint8_t SpawningPoolZergTier(std::int32_t dbfID) noexcept
{
    for (const auto& spec : SPAWNING_POOL_ZERG_BEHAVIORS)
        if (spec.dbfID == dbfID) return spec.tier;
    return 0;
}

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
        START_COMBAT_GLOBAL_STATS,
        END_TURN_TIER_THREE_STATS,
        END_TURN_EXTRA_TRIGGER,
        START_TURN_RANDOM_CARDS,
        COOKED_BOOK,
        TEAL_TIGER_SAPPHIRE,
        ALTER_EGO,
        MENAGERIE_MAYHEM,
        HIDDEN_TREASURE_VAULT,
        VOLATILE_VENOM,
        BLOOD_GOBLET,
        SINFALL_MEDALLION,
        ANIMA_BRIBE,
        VICTIMS_SPECTER,
        DEVILS_IN_DETAILS,
        PILFERED_LAMPS,
        KIDNAP_SACK,
        ANOTHER_HIDDEN_BODY,
        ETHEREAL_EVIDENCE,
        YOGG_TASTIES,
        FRIENDS_ALONG_THE_WAY,
        GHASTLY_MASK,
        UNMURLOC_YOUR_POTENTIAL,
        PARTNER_IN_CRIME,
        WONDROUS_WISDOMBALL,
        ESSENCE_OF_ZERUS,
        ENHANCE_A_MATIC,
        GOLDEN_HAMMER,
        STURDY_SHARD,
        BLOODSOAKED_TOME,
        ENDLESS_BLOOD_MOON,
        BEYOND_THE_MIRAGE,
        INVIGORATING_CONCH,
        TIMELINE_ACCELERATION,
        SMELTING_CHAMBER,
        STASH_OF_THE_SCRIBE,
        SPLITTING_SCROLL,
        DOUBLE_HEADED_REWARD,
        BOOM_SQUAD,
        CYCLE_ENERGY,
        TURBULENT_TOMBS,
        STABLE_AMALGAMATION,
        MAP_OF_THE_UNKNOWN,
        TEMPORAL_TAMPERING,
        NINE_LIVES,
        TOTEMIC_TAVERN,
        PURIFIED_SHARD,
        THE_WALL,
        RITUAL_DAGGER_REPEAT,
        BATTLECRY_REPEAT,
        AVENGE_REFRESH,
        START_TURN_RANDOM_SPELLS,
        TAVERN_EXTRA_MINIONS,
        REFRESH_GOLDEN_HIGHEST,
        RANDOM_TIER_SEVEN_COPY,
        OPPONENT_WARBAND_DISCOVER,
        COMBAT_SUMMON_BUFF,
        START_TURN_BUDDY_DISCOVER,
        START_COMBAT_RIGHTEOUS_CHARGE,
        RUSHING_WINDS,
        NORGANNON_REWARD,
        MAGICFIN_RELIC,
        UNTOLD_RICHES,
        GOLDEN_FORGE,
        QUAINT_BOUTIQUE,
        JUMBO_WAREHOUSE,
        COSMIC_REWARD,
        PERPETUAL_INCANTATION,
        RALLYING_CRY,
        OPPONENT_WARBAND_GUESS,
    } effect;
    bool executable = false;
    std::string_view missingLinkReason = {};
};

inline constexpr std::array<Season14GeneratedChoiceDefinition, 79>
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
        // Ghastly Mask's linked minion is derived from executable TURN_END
        // CardDefs and persisted when the reward is installed.
        {"BG24_Reward_130", 90917, Season14GeneratedChoiceDefinition::Effect::GHASTLY_MASK, true},
        {"BG24_Reward_131", 92542, Season14GeneratedChoiceDefinition::Effect::START_TURN_HAND_BUFF, true},
        {"BG24_Reward_136", 93069, Season14GeneratedChoiceDefinition::Effect::END_TURN_TIER_THREE_STATS, true},
        {"BG24_Reward_312", 92552, Season14GeneratedChoiceDefinition::Effect::START_COMBAT_GLOBAL_STATS, true},
        // The {0} placeholder is pinned to the lobby race pool at reward
        // installation; Player persists the selected race for replay.
        {"BG24_Reward_134", 92551, Season14GeneratedChoiceDefinition::Effect::FRIENDS_ALONG_THE_WAY, true},
        {"BG24_Reward_306", 89650, Season14GeneratedChoiceDefinition::Effect::COOKED_BOOK, true},
        {"BG24_Reward_308", 90435, Season14GeneratedChoiceDefinition::Effect::TEAL_TIGER_SAPPHIRE, true},
        {"BG24_Reward_321", 95149, Season14GeneratedChoiceDefinition::Effect::ALTER_EGO, true},
        // Linked reward token emitted by Alter Ego after its parity swap.
        // Keep it typed so replay can install the odd-tier variant without
        // treating the generated token as an unknown choice.
        {"BG24_Reward_321t", 96137, Season14GeneratedChoiceDefinition::Effect::ALTER_EGO, true},
        {"BG24_Reward_331", 95631, Season14GeneratedChoiceDefinition::Effect::MENAGERIE_MAYHEM, true},
        {"BG24_Reward_361", 97442, Season14GeneratedChoiceDefinition::Effect::HIDDEN_TREASURE_VAULT, true},
        {"BG24_Reward_364", 97597, Season14GeneratedChoiceDefinition::Effect::VOLATILE_VENOM, true},
        {"BG24_Reward_708", 97118, Season14GeneratedChoiceDefinition::Effect::BLOOD_GOBLET, true},
        {"BG24_Reward_712", 97452, Season14GeneratedChoiceDefinition::Effect::SINFALL_MEDALLION, true},
        {"BG24_Reward_305", 89645, Season14GeneratedChoiceDefinition::Effect::ANIMA_BRIBE, true},
        {"BG24_Reward_138", 93074, Season14GeneratedChoiceDefinition::Effect::VICTIMS_SPECTER, true},
        {"BG24_Reward_309", 90437, Season14GeneratedChoiceDefinition::Effect::DEVILS_IN_DETAILS, true},
        {"BG24_Reward_350", 95867, Season14GeneratedChoiceDefinition::Effect::PILFERED_LAMPS, true},
        {"BG24_Reward_718", 97966, Season14GeneratedChoiceDefinition::Effect::KIDNAP_SACK, true},
        {"BG24_Reward_311", 91992, Season14GeneratedChoiceDefinition::Effect::ANOTHER_HIDDEN_BODY, true},
        // Ethereal Evidence is executable through the generic reward-replace
        // modal. Yogg and Un-Murloc remain fail-closed because their reward
        // records are absent from the pinned Patch 36.4 manifest; their typed
        // identities are retained so replay cannot silently substitute another
        // effect.
        {"BG24_Reward_363", 97485, Season14GeneratedChoiceDefinition::Effect::ETHEREAL_EVIDENCE, true},
        {"BG24_Reward_135", 92563, Season14GeneratedChoiceDefinition::Effect::YOGG_TASTIES, false, "wheel_outcomes_unpinned"},
        {"BG24_Reward_535", 96151, Season14GeneratedChoiceDefinition::Effect::UNMURLOC_YOUR_POTENTIAL, false, "reward_absent_from_pinned_manifest"},
        {"BG24_Reward_310", 91980, Season14GeneratedChoiceDefinition::Effect::PARTNER_IN_CRIME, true},
        {"BG24_Reward_313", 92554, Season14GeneratedChoiceDefinition::Effect::WONDROUS_WISDOMBALL, true},
        {"BG24_Reward_362", 97445, Season14GeneratedChoiceDefinition::Effect::ESSENCE_OF_ZERUS, true},
        {"BG24_Reward_715", 97658, Season14GeneratedChoiceDefinition::Effect::ENHANCE_A_MATIC, true},
        {"BG24_Reward_719", 97756, Season14GeneratedChoiceDefinition::Effect::GOLDEN_HAMMER, true},
        {"BG27_Reward_804", 104675, Season14GeneratedChoiceDefinition::Effect::STURDY_SHARD, true},
        {"BG27_Reward_811", 104818, Season14GeneratedChoiceDefinition::Effect::BLOODSOAKED_TOME, true},
        {"BG27_Reward_815", 104842, Season14GeneratedChoiceDefinition::Effect::ENDLESS_BLOOD_MOON, true},
        {"BG28_Reward_500", 110298, Season14GeneratedChoiceDefinition::Effect::BEYOND_THE_MIRAGE, true},
        {"BG27_Reward_503", 104703, Season14GeneratedChoiceDefinition::Effect::INVIGORATING_CONCH, true},
        {"BG27_Reward_504", 104724, Season14GeneratedChoiceDefinition::Effect::TIMELINE_ACCELERATION, true},
        {"BG28_Reward_509", 110309, Season14GeneratedChoiceDefinition::Effect::SMELTING_CHAMBER, true},
        {"BG28_Reward_515", 110345, Season14GeneratedChoiceDefinition::Effect::STASH_OF_THE_SCRIBE, true},
        {"BG28_Reward_502", 110300, Season14GeneratedChoiceDefinition::Effect::SPLITTING_SCROLL, true},
        {"BG28_Reward_506", 110305, Season14GeneratedChoiceDefinition::Effect::DOUBLE_HEADED_REWARD, true},
        {"BG27_Reward_502", 104697, Season14GeneratedChoiceDefinition::Effect::BOOM_SQUAD, true},
        {"BG28_Reward_504", 110303, Season14GeneratedChoiceDefinition::Effect::CYCLE_ENERGY, true},
        {"BG27_Reward_803", 104670, Season14GeneratedChoiceDefinition::Effect::TURBULENT_TOMBS, true},
        {"BG28_Reward_518", 110551, Season14GeneratedChoiceDefinition::Effect::STABLE_AMALGAMATION, true},
        {"BG27_Reward_810", 104701, Season14GeneratedChoiceDefinition::Effect::MAP_OF_THE_UNKNOWN, true},
        {"BG28_Reward_501", 110299, Season14GeneratedChoiceDefinition::Effect::TEMPORAL_TAMPERING, true},
        // These five rows were present in the pinned card data but previously
        // had no executable lifecycle contract.
        // The ALT row has the same display name as Ritual Dagger but a
        // different pinned contract: the first Deathrattle each combat
        // triggers an extra time. Keep it distinct from BG24_Reward_113,
        // which grants +4/+4 after a friendly Deathrattle minion dies.
        {"BG24_Reward_113_ALT", 95858, Season14GeneratedChoiceDefinition::Effect::RITUAL_DAGGER_REPEAT, true},
        {"BG24_Reward_323", 96148, Season14GeneratedChoiceDefinition::Effect::NINE_LIVES, true},
        {"BG24_Reward_351", 96149, Season14GeneratedChoiceDefinition::Effect::TOTEMIC_TAVERN, true},
        {"BG24_Reward_352", 96150, Season14GeneratedChoiceDefinition::Effect::PURIFIED_SHARD, true},
        {"BG24_Reward_360", 97436, Season14GeneratedChoiceDefinition::Effect::THE_WALL, true},
        // Gilnean War Horn contains a server-selected `{0}` Battlecry
        // minion.  Patch 36.4's card snapshot does not carry that linked
        // quest payload, and choosing an arbitrary current-board Battlecry
        // would credit the wrong target.  Keep the typed identity visible,
        // but fail closed until the parent quest/replay payload is modeled.
        {"BG27_Reward_802", 104673, Season14GeneratedChoiceDefinition::Effect::BATTLECRY_REPEAT, false, "linked_battlecry_target_unpinned"},
        {"BG28_Reward_514", 110343, Season14GeneratedChoiceDefinition::Effect::START_TURN_RANDOM_SPELLS, true},
        {"BG33_Reward_004", 122015, Season14GeneratedChoiceDefinition::Effect::AVENGE_REFRESH, true},
        {"BG27_Reward_812", 104821, Season14GeneratedChoiceDefinition::Effect::TAVERN_EXTRA_MINIONS, true},
        {"BG28_Reward_508", 110308, Season14GeneratedChoiceDefinition::Effect::REFRESH_GOLDEN_HIGHEST, true},
        {"BG28_Reward_510", 110310, Season14GeneratedChoiceDefinition::Effect::RANDOM_TIER_SEVEN_COPY, true},
        {"BG27_Reward_806", 104677, Season14GeneratedChoiceDefinition::Effect::OPPONENT_WARBAND_DISCOVER, true},
        {"BG28_Reward_505", 110304, Season14GeneratedChoiceDefinition::Effect::COMBAT_SUMMON_BUFF, true},
        {"BG28_Reward_513", 110325, Season14GeneratedChoiceDefinition::Effect::START_TURN_BUDDY_DISCOVER, true},
        {"BG33_Reward_003", 122826, Season14GeneratedChoiceDefinition::Effect::START_COMBAT_RIGHTEOUS_CHARGE, true},
        {"BG33_Reward_006", 122820, Season14GeneratedChoiceDefinition::Effect::RUSHING_WINDS, true},
        {"BG33_Reward_010", 122822, Season14GeneratedChoiceDefinition::Effect::NORGANNON_REWARD, true},
        {"BG33_Reward_011", 122825, Season14GeneratedChoiceDefinition::Effect::MAGICFIN_RELIC, true},
        {"BG33_Reward_012", 121945, Season14GeneratedChoiceDefinition::Effect::UNTOLD_RICHES, true},
        {"BG33_Reward_013", 121943, Season14GeneratedChoiceDefinition::Effect::GOLDEN_FORGE, true},
        {"BG33_Reward_014", 122013, Season14GeneratedChoiceDefinition::Effect::QUAINT_BOUTIQUE, true},
        {"BG33_Reward_015", 122014, Season14GeneratedChoiceDefinition::Effect::JUMBO_WAREHOUSE, true},
        {"BG33_Reward_017", 122924, Season14GeneratedChoiceDefinition::Effect::COSMIC_REWARD, true},
        {"BG27_Anomaly_555t", 106440, Season14GeneratedChoiceDefinition::Effect::OPPONENT_WARBAND_GUESS, true},
        {"BG33_Reward_020", 121335, Season14GeneratedChoiceDefinition::Effect::PERPETUAL_INCANTATION, true},
        {"BG33_Reward_021", 122905, Season14GeneratedChoiceDefinition::Effect::RALLYING_CRY, true},
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
inline constexpr std::array<Season14HeroPowerDefinition, 46>
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
        {"BG31_HERO_811p", 120362,
         Season14HeroPowerKind::SPAWNING_POOL, 6, true},
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
