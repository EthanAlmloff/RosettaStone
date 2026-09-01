// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_BUDDY_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BUDDY_BEHAVIORS_HPP
#include <array>
#include <cstdint>
#include <string_view>
namespace RosettaStone::Battlegrounds {
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
 BuddyBloodGemDefinition{"BG20_HERO_103_Buddy_G",77479,2},
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
}
#endif
