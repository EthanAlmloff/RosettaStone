// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH9_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH9_HPP

#include <array>
#include <algorithm>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! The remaining non-Timewarp Season 14 powers whose public lifecycle is
//! driven by a reusable choice/turn boundary.  This registry deliberately
//! contains no guessed card pool: callers must provide the pinned offering
//! snapshot before consuming an activation.
enum class Season14HeroPowerBatch9Kind : std::uint8_t
{
    WHODUNIT,
    BUILD_AN_UNDEAD,
    PRESTIDIGITATION,
    ADVENTURE,
    BOBS_BURGLES,
    KELTHUZADS_KITTY,
    FRIENDLY_WAGER,
    PRIZE_WALL,
};

struct Season14HeroPowerBatch9Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch9Kind kind;
    std::int32_t cost;
    bool passive;
};

inline constexpr std::array<Season14HeroPowerBatch9Definition, 8>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH9 = {{
        {"BG24_HERO_100p", 92961, Season14HeroPowerBatch9Kind::WHODUNIT, 0, true},
        // HearthstoneJSON 36.4 pins Build-An-Undead at 3 gold.  Keeping the
        // cost here is important because SelectHero installs the native
        // payment amount from this registry (the hero-power card itself is
        // metadata-only in RosettaStone).
        {"BG25_HERO_100p", 97814, Season14HeroPowerBatch9Kind::BUILD_AN_UNDEAD, 3, false},
        {"TB_BaconShop_HP_020", 58022, Season14HeroPowerBatch9Kind::PRESTIDIGITATION, 1, false},
        {"TB_BaconShop_HP_057", 60450, Season14HeroPowerBatch9Kind::ADVENTURE, 0, true},
        {"TB_BaconShop_HP_077", 63162, Season14HeroPowerBatch9Kind::BOBS_BURGLES, 1, false},
        {"TB_BaconShop_HP_080", 63320, Season14HeroPowerBatch9Kind::KELTHUZADS_KITTY, 0, true},
        {"TB_BaconShop_HP_081", 63600, Season14HeroPowerBatch9Kind::FRIENDLY_WAGER, 1, false},
        {"TB_BaconShop_HP_106", 67357, Season14HeroPowerBatch9Kind::PRIZE_WALL, 0, true},
    }};

constexpr const Season14HeroPowerBatch9Definition*
FindSeason14HeroPowerBehaviorBatch9(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH9)
        if (definition.dbfID == dbfID) return &definition;
    return nullptr;
}

constexpr const Season14HeroPowerBatch9Definition*
FindSeason14HeroPowerBehaviorBatch9(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH9)
        if (definition.id == id) return &definition;
    return nullptr;
}

struct Season14HeroPowerBatch9State
{
    std::int32_t recruitTurnNumber = 0;
    std::int32_t prizeWallTurns = 0;
    std::int32_t creationsRemaining = 3;
    bool startGameChoiceReady = false;
    bool prizeChoiceReady = false;
    bool wagerPending = false;
    std::int32_t wagerTargetPlayer = -1;
    std::int32_t wagerResolutionCount = 0;
    std::array<std::int32_t, 5> activeSecretDbfIDs{};
    std::uint8_t activeSecretCount = 0;
    // Number of Prestidigitation Secret selections still to resolve after a
    // single paid activation (Ancient Wishbone can make this greater than 1).
    std::int32_t pendingPrestidigitationRemaining = 0;
    bool activationPending = false;
    // Build-An-Undead opens Pool 1 first and Pool 2 after the first
    // selection.  This is explicit state: a generic Undead scan is not the
    // authoritative offering pool.
    bool buildAnUndeadSecondPool = false;
};

// Patch 36.4's two fixed Build-An-Undead pools.  These are the normal DBF
// identities (never golden/generated derivatives).  The lists intentionally
// include cards that are not in the ordinary Tavern pool, as the hero power's
// custom pool is separate from the live minion pool.
inline constexpr std::array<std::int32_t, 26> BUILD_AN_UNDEAD_POOL_1 = {{
    99156, 104551, 99116, 99117, 99118, 99167, 113913, 96780, 126947,
    122485, 99200, 95251, 95265, 95255, 105440, 122481, 99166, 102340,
    122479, 126955, 95273, 104612, 120104, 99202, 95263, 99203}};

inline constexpr std::array<std::int32_t, 15> BUILD_AN_UNDEAD_POOL_2 = {{
    98735, 95246, 99578, 99159, 99577, 99160, 99201, 99161, 99524,
    95253, 99162, 103579, 102938, 99527, 98867}};

constexpr bool IsBuildAnUndeadPoolDbfID(std::int32_t dbfID,
                                        bool secondPool) noexcept
{
    const auto& pool = secondPool ? BUILD_AN_UNDEAD_POOL_2
                                  : BUILD_AN_UNDEAD_POOL_1;
    return std::find(pool.begin(), pool.end(), dbfID) != pool.end();
}

enum class Season14HeroPowerBatch9Event : std::uint8_t
{
    BEGIN_GAME,
    BEGIN_TURN,
    PLAYER_DIED,
};

struct Season14HeroPowerBatch9Result
{
    bool beginChoice = false;
    bool prizeReady = false;
    bool playerWarbandReady = false;
    bool wagerReady = false;
};

//! Turn/lobby lifecycle only.  Generated offerings are never fabricated here.
constexpr void ResolveSeason14HeroPowerBatch9Event(
    std::int32_t dbfID, Season14HeroPowerBatch9Event event,
    Season14HeroPowerBatch9State& state,
    Season14HeroPowerBatch9Result& result) noexcept
{
    result = {};
    const auto* definition = FindSeason14HeroPowerBehaviorBatch9(dbfID);
    if (definition == nullptr) return;
    if (event == Season14HeroPowerBatch9Event::BEGIN_GAME)
    {
        state.startGameChoiceReady =
            definition->kind == Season14HeroPowerBatch9Kind::WHODUNIT ||
            definition->kind == Season14HeroPowerBatch9Kind::ADVENTURE;
        result.beginChoice = state.startGameChoiceReady;
        return;
    }
    if (event == Season14HeroPowerBatch9Event::BEGIN_TURN)
    {
        ++state.recruitTurnNumber;
        if (definition->kind == Season14HeroPowerBatch9Kind::PRIZE_WALL)
        {
            if (++state.prizeWallTurns == 4)
            {
                state.prizeWallTurns = 0;
                state.prizeChoiceReady = true;
                result.prizeReady = true;
            }
        }
        return;
    }
    if (event == Season14HeroPowerBatch9Event::PLAYER_DIED &&
        definition->kind == Season14HeroPowerBatch9Kind::KELTHUZADS_KITTY)
        result.playerWarbandReady = true;
}

//! Friendly Wager is a sequential choice: payment commits the wager, and the
//! guessed seat is resolved only when the next combat result is known.
constexpr bool ArmSeason14FriendlyWager(Season14HeroPowerBatch9State& state) noexcept
{
    if (state.wagerPending) return false;
    state.wagerPending = true;
    state.wagerResolutionCount = 1;
    return true;
}

constexpr bool ResolveSeason14FriendlyWager(
    Season14HeroPowerBatch9State& state, bool won) noexcept
{
    if (!state.wagerPending) return false;
    state.wagerPending = false;
    state.wagerResolutionCount = 0;
    return won;
}

constexpr bool ConsumeSeason14PrizeWall(
    Season14HeroPowerBatch9State& state) noexcept
{
    if (!state.prizeChoiceReady) return false;
    state.prizeChoiceReady = false;
    return true;
}

constexpr bool ConsumeSeason14UndeadCreation(
    Season14HeroPowerBatch9State& state) noexcept
{
    if (state.creationsRemaining <= 0) return false;
    --state.creationsRemaining;
    return true;
}

//! Reserve a generated choice before constructing a public modal.  The
//! reservation is restored when pool construction fails, preserving Wishbone
//! replay semantics and preventing payment for an empty offering.
constexpr bool ConsumeSeason14HeroPowerBatch9Choice(
    Season14HeroPowerBatch9State& state) noexcept
{
    if (state.activationPending) return false;
    state.activationPending = true;
    return true;
}

constexpr void RestoreSeason14HeroPowerBatch9Choice(
    Season14HeroPowerBatch9State& state) noexcept
{
    state.activationPending = false;
}

constexpr bool CommitSeason14HeroPowerBatch9Choice(
    Season14HeroPowerBatch9State& state) noexcept
{
    if (!state.activationPending) return false;
    state.activationPending = false;
    return true;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH9_HPP
