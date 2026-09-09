// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_BUDDY_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BUDDY_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>
namespace RosettaStone::Battlegrounds {
// Lifecycle-owned legacy buddies have no fixed CardDef task graph. Their
// runtime owners are Player/Game/Battle; this reviewed identity registry
// keeps them distinct from metadata-only rows.
struct BuddyLegacyDefinition { std::string_view id; std::int32_t dbfID; };
inline constexpr std::array BUDDY_LEGACY_BEHAVIORS = {
 BuddyLegacyDefinition{"TB_BaconShop_HERO_02_Buddy",77494},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_02_Buddy_G",77730},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_08_Buddy",77626},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_08_Buddy_G",77731},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_11_Buddy",77821},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_11_Buddy_G",77822},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_12_Buddy",77843},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_12_Buddy_G",77844},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_17_Buddy",77805},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_17_Buddy_G",77806},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_57_Buddy",77512},
 BuddyLegacyDefinition{"TB_BaconShop_HERO_57_Buddy_G",77549},
};
struct BuddyDefinition { std::string_view id; std::int32_t dbfID; std::string_view heroID; int attackPerThreshold; };
inline constexpr std::array BUDDY_BEHAVIORS = {
 BuddyDefinition{"BG32_HERO_001_Buddy",122335,"BG32_HERO_001",1},
 BuddyDefinition{"BG32_HERO_001_Buddy_G",122336,"BG32_HERO_001",2},
 BuddyDefinition{"BG23_HERO_201_Buddy",98635,"BG23_HERO_201",2},
 BuddyDefinition{"BG23_HERO_201_Buddy_G",98636,"BG23_HERO_201",3},
 BuddyDefinition{"BG21_HERO_020_Buddy",77872,"BG21_HERO_020",1},
 BuddyDefinition{"BG21_HERO_020_Buddy_G",77873,"BG21_HERO_020",2},
 BuddyDefinition{"BG21_HERO_000_Buddy",77778,"BG21_HERO_000",1},
 BuddyDefinition{"BG21_HERO_000_Buddy_G",77779,"BG21_HERO_000",2},
 BuddyDefinition{"BG31_HERO_802_Buddy",122329,"BG31_HERO_802",0},
 BuddyDefinition{"BG31_HERO_802_Buddy_G",122330,"BG31_HERO_802",0},
};

// Arfus is resolved by the combat reborn boundary (not a generic trigger
// task), so keep its exact normal/golden transfer multiplier in the reviewed
// registry alongside the executable CardDef registrations.
struct BuddyRebornAttackDefinition { std::string_view id; std::int32_t dbfID; int attackMultiplier; };
inline constexpr std::array BUDDY_REBORN_ATTACK_BEHAVIORS = {
 BuddyRebornAttackDefinition{"TB_BaconShop_HERO_22_Buddy",77841,1},
 BuddyRebornAttackDefinition{"TB_BaconShop_HERO_22_Buddy_G",77842,2},
};

// Sub Scrubber's AFTER_PLAY_MINION trigger is represented by an explicit
// reusable enchantment (3/3, doubled while golden).
struct BuddyAfterMechPlayDefinition { std::string_view id; std::int32_t dbfID; int attackHealth; };
inline constexpr std::array BUDDY_AFTER_MECH_PLAY_BEHAVIORS = {
 BuddyAfterMechPlayDefinition{"BG22_HERO_200_Buddy",82600,3},
 BuddyAfterMechPlayDefinition{"BG22_HERO_200_Buddy_G",82603,6},
};

struct BuddyDeathSummonRaceDefinition { std::string_view id; std::int32_t dbfID; Race race; int summonCount; };
inline constexpr std::array BUDDY_DEATH_SUMMON_RACE_BEHAVIORS = {
 BuddyDeathSummonRaceDefinition{"TB_BaconShop_HERO_702_Buddy",98690,Race::UNDEAD,2},
 BuddyDeathSummonRaceDefinition{"TB_BaconShop_HERO_702_Buddy_G",98693,Race::UNDEAD,4},
};

struct BuddyRallyHandRaceDefinition { std::string_view id; std::int32_t dbfID; Race race; int copies; };
inline constexpr std::array BUDDY_RALLY_HAND_RACE_BEHAVIORS = {
 BuddyRallyHandRaceDefinition{"TB_BaconShop_HERO_56_Buddy",77448,Race::DRAGON,1},
 BuddyRallyHandRaceDefinition{"TB_BaconShop_HERO_56_Buddy_G",77533,Race::DRAGON,2},
};

struct BuddyAfterSellStatsDefinition { std::string_view id; std::int32_t dbfID; int attackHealth; };
inline constexpr std::array BUDDY_AFTER_SELL_STATS_BEHAVIORS = {
 BuddyAfterSellStatsDefinition{"TB_BaconShop_HERO_36_Buddy",77477,1},
 BuddyAfterSellStatsDefinition{"TB_BaconShop_HERO_36_Buddy_G",77608,2},
};

struct BuddyTauntSummonStatsDefinition { std::string_view id; std::int32_t dbfID; int attackHealth; };
inline constexpr std::array BUDDY_TAUNT_SUMMON_STATS_BEHAVIORS = {
 BuddyTauntSummonStatsDefinition{"TB_BaconShop_HERO_95_Buddy",77502,2},
 BuddyTauntSummonStatsDefinition{"TB_BaconShop_HERO_95_Buddy_G",77545,4},
};

struct BuddyGoldCoinDefinition { std::string_view id; std::int32_t dbfID; int gold; };
inline constexpr std::array BUDDY_GOLD_COIN_BEHAVIORS = {
 BuddyGoldCoinDefinition{"TB_BaconShop_HERO_72_Buddy",77511,1},
 BuddyGoldCoinDefinition{"TB_BaconShop_HERO_72_Buddy_G",77547,2},
};

//! Combat-kill health bonuses for Icesnarl. This is intentionally separate
//! from attackPerThreshold: the latter describes unrelated Buddy families.
struct BuddyCombatKillDefinition { std::string_view id; std::int32_t dbfID; int healthPerKill; };
inline constexpr std::array BUDDY_COMBAT_KILL_BEHAVIORS = {
 BuddyCombatKillDefinition{"BG20_HERO_100_Buddy",77876,1},
 BuddyCombatKillDefinition{"BG20_HERO_100_Buddy_G",77877,2},
};

struct BuddyEqualStatsPlayDefinition { std::string_view id; std::int32_t dbfID; int attackHealth; };
inline constexpr std::array BUDDY_EQUAL_STATS_PLAY_BEHAVIORS = {
 BuddyEqualStatsPlayDefinition{"BG20_HERO_101_Buddy",77851,1},
 BuddyEqualStatsPlayDefinition{"BG20_HERO_101_Buddy_G",77852,2},
};

struct BuddyBloodGemDefinition { std::string_view id; std::int32_t dbfID; int extraPerGem; };
inline constexpr std::array BUDDY_BLOOD_GEM_BEHAVIORS = {
 BuddyBloodGemDefinition{"BG20_HERO_103_Buddy",77478,1},
 BuddyBloodGemDefinition{"BG20_HERO_103_Buddy_G",77536,2},
};

struct BuddyHordeDefinition { std::string_view id; std::int32_t dbfID; int healthMultiplier; };
inline constexpr std::array BUDDY_HORDE_BEHAVIORS = {
 BuddyHordeDefinition{"BG20_HERO_102_Buddy",77813,1},
 BuddyHordeDefinition{"BG20_HERO_102_Buddy_G",77814,2},
};

struct BuddyAdjacentAttackDefinition { std::string_view id; std::int32_t dbfID; bool bothAdjacent; };
inline constexpr std::array BUDDY_ADJACENT_ATTACK_BEHAVIORS = {
 BuddyAdjacentAttackDefinition{"BG20_HERO_201_Buddy",77849,false},
 BuddyAdjacentAttackDefinition{"BG20_HERO_201_Buddy_G",77850,true},
};

struct BuddyFlightpathDefinition { std::string_view id; std::int32_t dbfID; int turnsPerEndTurn; };
inline constexpr std::array BUDDY_FLIGHTPATH_BEHAVIORS = {
 BuddyFlightpathDefinition{"BG20_HERO_283_Buddy",77788,1},
 BuddyFlightpathDefinition{"BG20_HERO_283_Buddy_G",77789,2},
};

struct BuddyDeathAttackDefinition { std::string_view id; std::int32_t dbfID; int attackMultiplier; };
inline constexpr std::array BUDDY_DEATH_ATTACK_BEHAVIORS = {
 BuddyDeathAttackDefinition{"BG20_HERO_282_Buddy",77878,1},
 BuddyDeathAttackDefinition{"BG20_HERO_282_Buddy_G",77879,2},
};

struct BuddyDevourDefinition { std::string_view id; std::int32_t dbfID; int extraTargets; };
inline constexpr std::array BUDDY_DEVOUR_BEHAVIORS = {
 BuddyDevourDefinition{"BG20_HERO_301_Buddy",77809,2},
 BuddyDevourDefinition{"BG20_HERO_301_Buddy_G",77810,4},
};

struct BuddyStormChoiceDefinition { std::string_view id; std::int32_t dbfID; int optionCount; };
inline constexpr std::array BUDDY_STORM_CHOICE_BEHAVIORS = {
 BuddyStormChoiceDefinition{"BG20_HERO_202_Buddy",77514,3},
 BuddyStormChoiceDefinition{"BG20_HERO_202_Buddy_G",77515,4},
};

struct BuddyTavernAfterBuyDefinition { std::string_view id; std::int32_t dbfID; int statBonus; };
inline constexpr std::array BUDDY_TAVERN_AFTER_BUY_BEHAVIORS = {
 BuddyTavernAfterBuyDefinition{"BG20_HERO_280_Buddy",77796,2},
 BuddyTavernAfterBuyDefinition{"BG20_HERO_280_Buddy_G",77797,4},
};

struct BuddyAvengeDefinition { std::string_view id; std::int32_t dbfID; int attack; int health; };
inline constexpr std::array BUDDY_AVENGE_BEHAVIORS = {
 BuddyAvengeDefinition{"BG22_HERO_002_Buddy",77884,1,0},
 BuddyAvengeDefinition{"BG22_HERO_002_Buddy_G",77885,2,0},
 BuddyAvengeDefinition{"BG22_HERO_003_Buddy",77886,0,1},
 BuddyAvengeDefinition{"BG22_HERO_003_Buddy_G",77887,0,2},
};

struct BuddyOpponentCopyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_OPPONENT_COPY_BEHAVIORS = {
 BuddyOpponentCopyDefinition{"BG21_HERO_010_Buddy",77870,1},
 BuddyOpponentCopyDefinition{"BG21_HERO_010_Buddy_G",77871,2},
};

struct BuddyAfterBuyStatsDefinition { std::string_view id; std::int32_t dbfID; int statBonus; };
inline constexpr std::array BUDDY_AFTER_BUY_STATS_BEHAVIORS = {
 BuddyAfterBuyStatsDefinition{"TB_BaconShop_HERO_01_Buddy",77479,2},
 BuddyAfterBuyStatsDefinition{"TB_BaconShop_HERO_01_Buddy_G",77538,4},
};

struct BuddyTavernTierStatsDefinition { std::string_view id; std::int32_t dbfID; int tavernTier; int attack; int health; };
inline constexpr std::array BUDDY_TAVERN_TIER_STATS_BEHAVIORS = {
 BuddyTavernTierStatsDefinition{"TB_BaconShop_HERO_16_Buddy",77774,3,1,2},
 BuddyTavernTierStatsDefinition{"TB_BaconShop_HERO_16_Buddy_G",77775,3,2,4},
};

struct BuddyFrozenTavernCopyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_FROZEN_TAVERN_COPY_BEHAVIORS = {
 BuddyFrozenTavernCopyDefinition{"TB_BaconShop_HERO_27_Buddy",77724,1},
 BuddyFrozenTavernCopyDefinition{"TB_BaconShop_HERO_27_Buddy_G",77725,2},
};

struct BuddyTavernTierHandDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_TAVERN_TIER_HAND_BEHAVIORS = {
 BuddyTavernTierHandDefinition{"TB_BaconShop_HERO_74_Buddy",77493,1},
 BuddyTavernTierHandDefinition{"TB_BaconShop_HERO_74_Buddy_G",77541,2},
};

struct BuddyFriendlyTierBuffDefinition { std::string_view id; std::int32_t dbfID; int multiplier; };
inline constexpr std::array BUDDY_FRIENDLY_TIER_BUFF_BEHAVIORS = {
 BuddyFriendlyTierBuffDefinition{"TB_BaconShop_HERO_75_Buddy",77823,1},
 BuddyFriendlyTierBuffDefinition{"TB_BaconShop_HERO_75_Buddy_G",77824,2},
};

struct BuddyGoldenFriendlyBuffDefinition { std::string_view id; std::int32_t dbfID; int attack; int health; };
inline constexpr std::array BUDDY_GOLDEN_FRIENDLY_BUFF_BEHAVIORS = {
 BuddyGoldenFriendlyBuffDefinition{"TB_BaconShop_HERO_64_Buddy",77472,5,5},
 BuddyGoldenFriendlyBuffDefinition{"TB_BaconShop_HERO_64_Buddy_G",77535,10,10},
};

struct BuddyDeathrattleCopyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_DEATHRATTLE_COPY_BEHAVIORS = {
 BuddyDeathrattleCopyDefinition{"BG21_HERO_030_Buddy",77874,1},
 BuddyDeathrattleCopyDefinition{"BG21_HERO_030_Buddy_G",77875,2},
};

struct BuddyRefreshHighestCopyDefinition { std::string_view id; std::int32_t dbfID; int statMultiplier; };
inline constexpr std::array BUDDY_REFRESH_HIGHEST_COPY_BEHAVIORS = {
 BuddyRefreshHighestCopyDefinition{"BG22_HERO_004_Buddy",77888,1},
 BuddyRefreshHighestCopyDefinition{"BG22_HERO_004_Buddy_G",77889,2},
};

struct BuddyGoldSpentBattlecryDefinition { std::string_view id; std::int32_t dbfID; int multiplier; };
inline constexpr std::array BUDDY_GOLD_SPENT_BATTLECRY_BEHAVIORS = {
 BuddyGoldSpentBattlecryDefinition{"TB_BaconShop_HERO_10_Buddy",77847,1},
 BuddyGoldSpentBattlecryDefinition{"TB_BaconShop_HERO_10_Buddy_G",77848,2},
};

struct BuddyPiratesPlayedBattlecryDefinition { std::string_view id; std::int32_t dbfID; int multiplier; };
inline constexpr std::array BUDDY_PIRATES_PLAYED_BATTLECRY_BEHAVIORS = {
 BuddyPiratesPlayedBattlecryDefinition{"TB_BaconShop_HERO_18_Buddy",77815,1},
 BuddyPiratesPlayedBattlecryDefinition{"TB_BaconShop_HERO_18_Buddy_G",77816,2},
};
struct BuddyTierRefreshBattlecryDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_TIER_REFRESH_BATTLECRY_BEHAVIORS = {
 BuddyTierRefreshBattlecryDefinition{"BG20_HERO_242_Buddy",77790,1},
 BuddyTierRefreshBattlecryDefinition{"BG20_HERO_242_Buddy_G",77791,2},
};
struct BuddyHeroPowerDamageDefinition { std::string_view id; std::int32_t dbfID; int multiplier; };
inline constexpr std::array BUDDY_HERO_POWER_DAMAGE_BEHAVIORS = {
 BuddyHeroPowerDamageDefinition{"BG22_HERO_000_Buddy",77880,1},
 BuddyHeroPowerDamageDefinition{"BG22_HERO_000_Buddy_G",77881,2},
};
struct BuddyWhelpSummonDefinition { std::string_view id; std::int32_t dbfID; int attack; int health; };
inline constexpr std::array BUDDY_WHELP_SUMMON_BEHAVIORS = {
 BuddyWhelpSummonDefinition{"BG22_HERO_305_Buddy",77890,2,2},
 BuddyWhelpSummonDefinition{"BG22_HERO_305_Buddy_G",77891,4,4},
};
struct BuddyTierHandBattlecryDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_TIER_HAND_BATTLECRY_BEHAVIORS = {
 BuddyTierHandBattlecryDefinition{"BG22_HERO_201_Buddy",82604,1},
 BuddyTierHandBattlecryDefinition{"BG22_HERO_201_Buddy_G",82606,2},
};
struct BuddyBuyTierTavernDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_BUY_TIER_TAVERN_BEHAVIORS = {
 BuddyBuyTierTavernDefinition{"TB_BaconShop_HERO_49_Buddy",77803,1},
 BuddyBuyTierTavernDefinition{"TB_BaconShop_HERO_49_Buddy_G",77804,2},
};

// These Buddy battlecries are executable CardDefs in the token behavior
// registry.  Keep their exact normal/golden fan-out in the shared identity
// registry so coverage cannot mistake the CardDef-only registration for an
// unowned or metadata-only entity.
struct BuddyBattlecryCopiesDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_BATTLECRY_COPIES_BEHAVIORS = {
 BuddyBattlecryCopiesDefinition{"TB_BaconShop_HERO_23_Buddy",77827,1},
 BuddyBattlecryCopiesDefinition{"TB_BaconShop_HERO_23_Buddy_G",77828,2},
 BuddyBattlecryCopiesDefinition{"BG25_HERO_100_Buddy",101442,1},
 BuddyBattlecryCopiesDefinition{"BG25_HERO_100_Buddy_G",101443,2},
 BuddyBattlecryCopiesDefinition{"BG25_HERO_105_Buddy",101349,1},
 BuddyBattlecryCopiesDefinition{"BG25_HERO_105_Buddy_G",101350,2},
};

struct BuddyDifferentTypeStatsDefinition { std::string_view id; std::int32_t dbfID; int requiredTypes; int multiplier; };
inline constexpr std::array BUDDY_DIFFERENT_TYPE_STATS_BEHAVIORS = {
 BuddyDifferentTypeStatsDefinition{"TB_BaconShop_HERO_14_Buddy",77629,4,1},
 BuddyDifferentTypeStatsDefinition{"TB_BaconShop_HERO_14_Buddy_G",77732,4,2},
};

struct BuddyKeywordDeathrattleDefinition { std::string_view id; std::int32_t dbfID; int targets; };
inline constexpr std::array BUDDY_KEYWORD_DEATHRATTLE_BEHAVIORS = {
 BuddyKeywordDeathrattleDefinition{"TB_BaconShop_HERO_76_Buddy",77447,1},
 BuddyKeywordDeathrattleDefinition{"TB_BaconShop_HERO_76_Buddy_G",77532,2},
};

struct BuddyBattlecryDeathrattleDefinition { std::string_view id; std::int32_t dbfID; int handCopies; };
inline constexpr std::array BUDDY_BATTLECRY_DEATHRATTLE_BEHAVIORS = {
 BuddyBattlecryDeathrattleDefinition{"TB_BaconShop_HERO_43_Buddy",77784,1},
 BuddyBattlecryDeathrattleDefinition{"TB_BaconShop_HERO_43_Buddy_G",77785,2},
};

struct BuddyHeroPowerDivineShieldDefinition { std::string_view id; std::int32_t dbfID; int attack; };
inline constexpr std::array BUDDY_HERO_POWER_DIVINE_SHIELD_BEHAVIORS = {
 BuddyHeroPowerDivineShieldDefinition{"TB_BaconShop_HERO_15_Buddy",77495,2},
 BuddyHeroPowerDivineShieldDefinition{"TB_BaconShop_HERO_15_Buddy_G",77543,4},
};

//! Solemn Serenader observes a successful Hero Power that targeted a living
//! friendly minion.  Its payload is derived from each owned Buddy instance's
//! current Attack, so ordinary/golden copies and temporary stat changes are
//! resolved independently at the event boundary.
struct BuddyHeroPowerTargetDefinition {
    std::string_view id;
    std::int32_t dbfID;
    int attackDivisor;
};
inline constexpr std::array BUDDY_HERO_POWER_TARGET_BEHAVIORS = {
 BuddyHeroPowerTargetDefinition{"BG26_HERO_102_Buddy",113627,2},
 BuddyHeroPowerTargetDefinition{"BG26_HERO_102_Buddy_G",113628,1},
};

struct BuddyEndTurnLeftHealthDefinition { std::string_view id; std::int32_t dbfID; int adjacentTargets; };
inline constexpr std::array BUDDY_END_TURN_LEFT_HEALTH_BEHAVIORS = {
 BuddyEndTurnLeftHealthDefinition{"TB_BaconShop_HERO_34_Buddy",77817,1},
 BuddyEndTurnLeftHealthDefinition{"TB_BaconShop_HERO_34_Buddy_G",77818,2},
};

// These three pairs are resolved at phase/zone boundaries rather than by a
// CardDef task graph.  Keep their exact fan-out and boundary semantics in a
// typed registry so coverage can credit the executable Player/Game hooks.
struct BuddyOpponentBuddyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_OPPONENT_BUDDY_BEHAVIORS = {
 BuddyOpponentBuddyDefinition{"TB_BaconShop_HERO_50_Buddy",77835,1},
 BuddyOpponentBuddyDefinition{"TB_BaconShop_HERO_50_Buddy_G",77836,2},
};

struct BuddyLowestHealthOpponentDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_LOWEST_HEALTH_OPPONENT_BEHAVIORS = {
 BuddyLowestHealthOpponentDefinition{"TB_BaconShop_HERO_70_Buddy",77807,1},
 BuddyLowestHealthOpponentDefinition{"TB_BaconShop_HERO_70_Buddy_G",77808,2},
};

struct BuddyDiscoverTierDefinition { std::string_view id; std::int32_t dbfID; int tierDelta; int choices; };
inline constexpr std::array BUDDY_DISCOVER_TIER_BEHAVIORS = {
 BuddyDiscoverTierDefinition{"TB_BaconShop_HERO_28_Buddy",77507,1,1},
 BuddyDiscoverTierDefinition{"TB_BaconShop_HERO_28_Buddy_G",77603,1,2},
};

//! Burth is resolved at the Discover commit boundary.  The payload is
//! instance-owned: every owned normal/golden copy buffs the selected minion
//! once and advances only its own future payload.
struct BuddyDiscoverBuffDefinition {
 std::string_view id; std::int32_t dbfID; int attack; int health;
 int attackImprovement; int healthImprovement;
};
inline constexpr std::array BUDDY_DISCOVER_BUFF_BEHAVIORS = {
 BuddyDiscoverBuffDefinition{"TB_BaconShop_HERO_90_Buddy",77829,2,2,1,1},
 BuddyDiscoverBuffDefinition{"TB_BaconShop_HERO_90_Buddy_G",77830,4,4,1,1},
};

struct BuddyTransformTavernDefinition { std::string_view id; std::int32_t dbfID; int tierDelta; };
inline constexpr std::array BUDDY_TRANSFORM_TAVERN_BEHAVIORS = {
 BuddyTransformTavernDefinition{"TB_BaconShop_HERO_55_Buddy",77786,0},
 BuddyTransformTavernDefinition{"TB_BaconShop_HERO_55_Buddy_G",77787,1},
};

struct BuddyLeadExplorerDiscountDefinition { std::string_view id; std::int32_t dbfID; int discount; };
inline constexpr std::array BUDDY_LEAD_EXPLORER_DISCOUNT_BEHAVIORS = {
 BuddyLeadExplorerDiscountDefinition{"TB_BaconShop_HERO_42_Buddy",77484,2},
 BuddyLeadExplorerDiscountDefinition{"TB_BaconShop_HERO_42_Buddy_G",77540,4},
};

struct BuddySellRefreshHeroPowerDefinition { std::string_view id; std::int32_t dbfID; };
inline constexpr std::array BUDDY_SELL_REFRESH_HERO_POWER_BEHAVIORS = {
 BuddySellRefreshHeroPowerDefinition{"TB_BaconShop_HERO_68_Buddy",77833},
 BuddySellRefreshHeroPowerDefinition{"TB_BaconShop_HERO_68_Buddy_G",77834},
};

struct BuddyRefreshExtraOfferDefinition {
 std::string_view id; std::int32_t dbfID; int copies; bool elementalOnly; bool frozen;
};
inline constexpr std::array BUDDY_REFRESH_EXTRA_OFFER_BEHAVIORS = {
 BuddyRefreshExtraOfferDefinition{"TB_BaconShop_HERO_59_Buddy",77468,1,false,false},
 BuddyRefreshExtraOfferDefinition{"TB_BaconShop_HERO_59_Buddy_G",78112,2,false,false},
 BuddyRefreshExtraOfferDefinition{"TB_BaconShop_HERO_78_Buddy",77780,1,true,true},
 BuddyRefreshExtraOfferDefinition{"TB_BaconShop_HERO_78_Buddy_G",77781,2,true,true},
};

struct BuddyDarkmoonPrizeDefinition { std::string_view id; std::int32_t dbfID; int selections; };
inline constexpr std::array BUDDY_DARKMOON_PRIZE_BEHAVIORS = {
 BuddyDarkmoonPrizeDefinition{"TB_BaconShop_HERO_94_Buddy",77845,1},
 BuddyDarkmoonPrizeDefinition{"TB_BaconShop_HERO_94_Buddy_G",77846,2},
};

// Refresh/cast/purchase lifecycle Buddies whose predicates require the
// authoritative Player boundary rather than a static CardDef task graph.
struct BuddyRefreshRaceOfferDefinition {
 std::string_view id; std::int32_t dbfID; Race race; int offers;
};
inline constexpr std::array BUDDY_REFRESH_RACE_OFFER_BEHAVIORS = {
 BuddyRefreshRaceOfferDefinition{"BG26_HERO_101_Buddy",114598,Race::PIRATE,1},
 BuddyRefreshRaceOfferDefinition{"BG26_HERO_101_Buddy_G",114599,Race::PIRATE,2},
};

struct BuddyCastSpellCopyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_CAST_SPELL_COPY_BEHAVIORS = {
 BuddyCastSpellCopyDefinition{"BG28_HERO_800_Buddy",113650,1},
 BuddyCastSpellCopyDefinition{"BG28_HERO_800_Buddy_G",113651,2},
};

struct BuddyZeroCostSpellCopyDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_ZERO_COST_SPELL_COPY_BEHAVIORS = {
 BuddyZeroCostSpellCopyDefinition{"BG31_HERO_006_Buddy",122342,1},
 BuddyZeroCostSpellCopyDefinition{"BG31_HERO_006_Buddy_G",122343,2},
};

// Coilfang Elite watches fresh Tavern offers.  Each Spellcraft minion that
// appears grants one copy of its Spellcraft spell; the golden Buddy grants
// two.  The offer-to-hand transfer is resolved at the Player refresh boundary
// (where the Tavern is authoritative), while this typed table preserves the
// exact normal/golden identity and fan-out for coverage and replay audits.
struct BuddySpellcraftOfferDefinition { std::string_view id; std::int32_t dbfID; int copies; };
inline constexpr std::array BUDDY_SPELLCRAFT_OFFER_BEHAVIORS = {
 BuddySpellcraftOfferDefinition{"BG23_HERO_304_Buddy",101458,1},
 BuddySpellcraftOfferDefinition{"BG23_HERO_304_Buddy_G",101463,2},
};
}
#endif
