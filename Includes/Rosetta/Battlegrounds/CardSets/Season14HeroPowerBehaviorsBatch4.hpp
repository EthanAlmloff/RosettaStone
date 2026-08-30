// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH4_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH4_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Small, executable Season 14 hero-power families that do not need a target,
//! discover result, or random pool selection.  The bridge must still use the
//! event/activation resolvers below; registry membership alone is not enough
//! to make a power legal.
enum class Season14HeroPowerBatch4Kind : std::uint8_t
{
    NEXT_BUY_ATTACK,
    TAVERN_MECH_STATS,
    GLOBAL_MINION_ATTACK,
    THIRD_MINION_BUFF,
    SEVEN_TAVERN_SLOTS,
    GRAVEYARD_SHIFT,
    AVENGE_ATTACK,
    AVENGE_HEALTH,
};

struct Season14HeroPowerBatch4Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch4Kind kind;
    std::int32_t cost;
    bool passive;
};

//! Exact Patch 36.4 IDs in this focused, target-free batch.
inline constexpr std::array<Season14HeroPowerBatch4Definition, 8>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH4 = {{
        {"BG20_HERO_102p", 71455,
         Season14HeroPowerBatch4Kind::NEXT_BUY_ATTACK, 1, false},
        {"TB_BaconShop_HP_015", 57949,
         Season14HeroPowerBatch4Kind::TAVERN_MECH_STATS, 0, true},
        {"TB_BaconShop_HP_061", 61406,
         Season14HeroPowerBatch4Kind::GLOBAL_MINION_ATTACK, 0, true},
        {"TB_BaconShop_HP_066", 61917,
         Season14HeroPowerBatch4Kind::THIRD_MINION_BUFF, 0, true},
        {"TB_BaconShop_HP_065t2", 62035,
         Season14HeroPowerBatch4Kind::SEVEN_TAVERN_SLOTS, 0, true},
        {"TB_BaconShop_HP_049", 60285,
         Season14HeroPowerBatch4Kind::GRAVEYARD_SHIFT, 2, false},
        {"BG22_HERO_002p", 80244,
         Season14HeroPowerBatch4Kind::AVENGE_ATTACK, 0, true},
        {"BG22_HERO_003p", 80248,
         Season14HeroPowerBatch4Kind::AVENGE_HEALTH, 0, true},
    }};

constexpr const Season14HeroPowerBatch4Definition*
FindSeason14HeroPowerBehaviorBatch4(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH4)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerBatch4Definition*
FindSeason14HeroPowerBehaviorBatch4(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH4)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

struct Season14HeroPowerBatch4State
{
    std::int32_t turnNumber = 0;
    std::int32_t nextBuyAttack = 0;
    std::int32_t minionsPlayedThisTurn = 0;
    std::int32_t combatDeaths = 0;
};

enum class Season14HeroPowerBatch4Event : std::uint8_t
{
    BEGIN_TURN,
    COMBAT_START,
    BUY_MINION,
    PLAY_MINION,
    FRIENDLY_MINION_DIED,
};

struct Season14HeroPowerBatch4Result
{
    std::int32_t goldDelta = 0;
    std::int32_t healthDelta = 0;
    std::int32_t attack = 0;
    std::int32_t health = 0;
    std::int32_t purchaseAttack = 0;
    bool avengeTriggered = false;
};

//! Passive, fixed modifiers used while constructing a fresh Tavern.
struct Season14HeroPowerBatch4PassiveModifiers
{
    std::int32_t globalMinionAttack = 0;
    std::int32_t mechShopAttack = 0;
    std::int32_t mechShopHealth = 0;
    std::int32_t tavernSlotsDelta = 0;
};

constexpr Season14HeroPowerBatch4PassiveModifiers
Season14HeroPowerBatch4Modifiers(std::int32_t dbfID) noexcept
{
    switch (dbfID)
    {
        case 57949: // Tinker
            return {.mechShopAttack = 1, .mechShopHealth = 1};
        case 61406: // ALL Will Burn!
            return {.globalMinionAttack = 3};
        case 62035: // Spectral Sight
            return {.tavernSlotsDelta = 1};
        default:
            return {};
    }
}

//! Resolve lifecycle events.  Avenge powers trigger after three friendly
//! combat deaths, while Verdant Spheres triggers on every third minion played.
//! The bridge never calls this resolver for an unsupported/targeted family.
constexpr void ResolveSeason14HeroPowerBatch4Event(
    std::int32_t dbfID, Season14HeroPowerBatch4Event event,
    Season14HeroPowerBatch4State& state,
    Season14HeroPowerBatch4Result& result) noexcept
{
    result = {};
    if (event == Season14HeroPowerBatch4Event::BEGIN_TURN)
    {
        ++state.turnNumber;
        state.minionsPlayedThisTurn = 0;
        // For the Horde! arms its next purchase only when the active power is
        // used.  A new turn still clears an unconsumed arm so a failed/unused
        // action cannot leak into a later turn.
        state.nextBuyAttack = 0;
        return;
    }

    if (event == Season14HeroPowerBatch4Event::COMBAT_START)
    {
        state.combatDeaths = 0;
        return;
    }

    if (event == Season14HeroPowerBatch4Event::BUY_MINION && dbfID == 71455)
    {
        result.purchaseAttack = state.nextBuyAttack;
        state.nextBuyAttack = 0;
        return;
    }

    if (event == Season14HeroPowerBatch4Event::PLAY_MINION && dbfID == 61917)
    {
        ++state.minionsPlayedThisTurn;
        if (state.minionsPlayedThisTurn == 3)
        {
            result.attack = 2;
            result.health = 2;
            state.minionsPlayedThisTurn = 0;
        }
        return;
    }

    if (event == Season14HeroPowerBatch4Event::FRIENDLY_MINION_DIED &&
        (dbfID == 80244 || dbfID == 80248))
    {
        ++state.combatDeaths;
        if (state.combatDeaths >= (dbfID == 80244 ? 3 : 2))
        {
            state.combatDeaths = 0;
            result.attack = dbfID == 80244 ? 1 : 0;
            result.health = dbfID == 80248 ? 1 : 0;
            result.avengeTriggered = true;
        }
    }
}

//! Resolve the one active target-free power in this batch.  Graveyard Shift
//! exchanges four hero Health for two Gold; payment/use remains atomic in the
//! Player/bridge layer.
constexpr bool ResolveSeason14HeroPowerBatch4Activation(
    std::int32_t dbfID, Season14HeroPowerBatch4State& state,
    Season14HeroPowerBatch4Result& result) noexcept
{
    result = {};
    if (dbfID == 71455) // For the Horde! starts at +2 and scales by turn.
    {
        state.nextBuyAttack = state.turnNumber + 1;
        return true;
    }
    if (dbfID != 60285)
    {
        return false;
    }
    result.goldDelta = 2;
    result.healthDelta = -4;
    return true;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH4_HPP
