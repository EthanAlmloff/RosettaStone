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
    TAVERN_SPELL_DISCOVER,
};

struct Season14HeroPowerDefinition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerKind kind;
    std::int32_t cost;
    bool passive;
};

//! Exact Patch 36.4 behavior batch (eight distinct reusable families).
inline constexpr std::array<Season14HeroPowerDefinition, 9>
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
         Season14HeroPowerKind::TAVERN_SPELL_DISCOVER, 1, false},
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

//! Deterministic player-owned modifiers for the supported passive families.
//!
//! The registry also contains target-dependent/random powers.  Those powers
//! intentionally leave these fields at their neutral values until their
//! target/random contract is implemented by the simulator.
struct Season14HeroPowerBatch1State
{
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
    bool consumesTurnUse = true;
};

//! Resolve the two no-target active powers in this batch.
//!
//! `turnNumber` is one-based: the first recruit turn returns +2 for Piggy
//! Bank, the second returns +3, and so on.  Passive and target-dependent
//! powers return false so callers cannot accidentally claim an incomplete
//! implementation.
constexpr bool ResolveSeason14HeroPowerActivation(
    std::int32_t dbfID, std::int32_t turnNumber,
    Season14HeroPowerActivation& result) noexcept
{
    result = {};
    if (dbfID == 62269) // TB_BaconShop_HP_076, Piggy Bank
    {
        result.goldDelta = 1 + (turnNumber > 0 ? turnNumber : 1);
        return true;
    }
    if (dbfID == 116921) // BG32_HERO_001p, Wisdom of Ancients
    {
        result.maxGoldDelta = 1;
        return true;
    }
    return false;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_HPP
