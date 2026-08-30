// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH2_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH2_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Explicit behavior families for the second Season 14 hero-power batch.
//!
//! The registry is deliberately independent of card prose and of the first
//! batch.  A caller must use the event resolver below; merely finding an ID
//! in this table does not make a target-dependent or random effect legal.
enum class Season14HeroPowerBatch2Kind : std::uint8_t
{
    DEFERRED_SELL_GOLD,
    BLOOD_GEMS,
    FROZEN_TAVERN_ECONOMY,
    HIGHER_TIER_REFRESH,
    UPGRADE_GOLD,
    ELEMENTAL_UPGRADE_DISCOUNT,
    LAST_SPELL_COPY,
    TAVERN_SPELL_REFRESH,
    TAVERN_SPELL_DISCOUNT,
    TWO_COPY_GOLDEN,
};

struct Season14HeroPowerBatch2Definition
{
    std::string_view id;
    std::int32_t dbfID;
    Season14HeroPowerBatch2Kind kind;
    std::int32_t cost;
    bool passive;
};

//! Exact Patch 36.4 IDs covered by this batch.
inline constexpr std::array<Season14HeroPowerBatch2Definition, 10>
    SEASON14_HERO_POWER_BEHAVIORS_BATCH2 = {{
        {"TB_BaconShop_HP_008", 57559,
         Season14HeroPowerBatch2Kind::DEFERRED_SELL_GOLD, 1, true},
        {"BG20_HERO_103p", 71459,
         Season14HeroPowerBatch2Kind::BLOOD_GEMS, 1, false},
        {"TB_BaconShop_HP_014", 57945,
         Season14HeroPowerBatch2Kind::FROZEN_TAVERN_ECONOMY, 0, true},
        {"TB_BaconShop_HP_028", 58537,
         Season14HeroPowerBatch2Kind::HIGHER_TIER_REFRESH, 1, false},
        {"TB_BaconShop_HP_082", 63605,
         Season14HeroPowerBatch2Kind::UPGRADE_GOLD, 0, true},
        {"TB_BaconShop_HP_088", 64476,
         Season14HeroPowerBatch2Kind::ELEMENTAL_UPGRADE_DISCOUNT, 0, true},
        {"BG31_HERO_003p", 116924,
         Season14HeroPowerBatch2Kind::LAST_SPELL_COPY, 3, false},
        {"BG34_HERO_001p", 126538,
         Season14HeroPowerBatch2Kind::TAVERN_SPELL_REFRESH, 0, true},
        {"BG31_HERO_006p", 117426,
         Season14HeroPowerBatch2Kind::TAVERN_SPELL_DISCOUNT, 0, true},
        {"BG34_HERO_002p", 126533,
         Season14HeroPowerBatch2Kind::TWO_COPY_GOLDEN, 0, true},
    }};

constexpr const Season14HeroPowerBatch2Definition*
FindSeason14HeroPowerBehaviorBatch2(std::int32_t dbfID) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH2)
    {
        if (definition.dbfID == dbfID)
        {
            return &definition;
        }
    }
    return nullptr;
}

constexpr const Season14HeroPowerBatch2Definition*
FindSeason14HeroPowerBehaviorBatch2(std::string_view id) noexcept
{
    for (const auto& definition : SEASON14_HERO_POWER_BEHAVIORS_BATCH2)
    {
        if (definition.id == id)
        {
            return &definition;
        }
    }
    return nullptr;
}

//! Persistent counters owned by a player for this batch.
struct Season14HeroPowerBatch2State
{
    std::int32_t turnNumber = 0;
    std::int32_t bloodboundUsesThisTurn = 0;
    std::int32_t deferredGoldNextTurn = 0;
    std::int32_t elementalPlays = 0;
    std::int32_t upgradeCostReduction = 0;
    std::int32_t tavernSpellDiscount = 0;
    //! One-shot count armed by Temporal Tavern for the next shop fill.
    std::int32_t higherTierRefreshMinions = 0;
    bool arcaneKnowledgeUnlocked = false;
    bool nextHeroPowerDiscount = false;

    //! Returns the effective cost of a Tavern spell before it resolves.
    constexpr std::int32_t TavernSpellCost(std::int32_t baseCost) const noexcept
    {
        return baseCost < tavernSpellDiscount
                   ? 0
                   : baseCost - tavernSpellDiscount;
    }

    //! Consumes one granted Tavern-spell discount after a successful spell.
    //!
    //! The caller must pass false for failed, unaffordable, or unsupported
    //! attempts.  Those attempts must not consume a one-shot discount.
    constexpr bool ConsumeTavernSpellDiscount(bool spellResolved) noexcept
    {
        if (!spellResolved || tavernSpellDiscount <= 0)
        {
            return false;
        }

        --tavernSpellDiscount;
        return true;
    }
};

//! Events that can advance a batch-2 lifecycle counter.
enum class Season14HeroPowerBatch2Event : std::uint8_t
{
    BEGIN_TURN,
    SELL_MINION,
    PLAY_ELEMENTAL,
    UPGRADE_TAVERN,
};

//! Passive modifiers that must be installed when a hero is selected.
struct Season14HeroPowerBatch2PassiveModifiers
{
    std::int32_t minionCost = 0;
    std::int32_t refreshCost = 0;
    std::int32_t extraHigherTierMinions = 0;
    std::int32_t tavernSlotsDelta = 0;
    bool freezeRemainingShopAtEnd = false;
    bool refreshWithTavernSpells = false;
    bool twoCopiesMakeGolden = false;
};

constexpr Season14HeroPowerBatch2PassiveModifiers
Season14HeroPowerBatch2Modifiers(std::int32_t dbfID) noexcept
{
    switch (dbfID)
    {
        case 57945: // Stay Frosty
            return {.minionCost = 2,
                    .refreshCost = 2,
                    .extraHigherTierMinions = 0,
                    .tavernSlotsDelta = -1,
                    .freezeRemainingShopAtEnd = true};
        case 126538: // Mana Per Minute
            return {.refreshWithTavernSpells = true};
        case 117426: // Arcane Knowledge is unlocked on turn 3; see resolver.
            return {};
        case 126533: // Double Time
            return {.twoCopiesMakeGolden = true};
        default:
            return {};
    }
}

//! Result of one deterministic active/event transition.
struct Season14HeroPowerBatch2Result
{
    std::int32_t goldDelta = 0;
    std::int32_t bloodGemDelta = 0;
    std::int32_t upgradeCostDelta = 0;
    std::int32_t heroPowerCostDelta = 0;
    std::int32_t spellCostDelta = 0;
    std::int32_t extraHigherTierMinions = 0;
    bool copyLastTavernSpell = false;
    bool refreshWithTavernSpells = false;
    bool twoCopiesMakeGolden = false;
};

//! Resolve a lifecycle event without silently implementing random/targeted
//! behavior.  `BEGIN_TURN` is one-based and pays Smart Savings' deferred gold.
constexpr void ResolveSeason14HeroPowerBatch2Event(
    std::int32_t dbfID, Season14HeroPowerBatch2Event event,
    Season14HeroPowerBatch2State& state,
    Season14HeroPowerBatch2Result& result) noexcept
{
    result = {};

    // Recruit begins are one-based.  Advance the lifecycle before resolving
    // start-of-turn effects so Arcane Knowledge unlocks on the third begin,
    // not the fourth.
    if (event == Season14HeroPowerBatch2Event::BEGIN_TURN)
    {
        ++state.turnNumber;
        state.bloodboundUsesThisTurn = 0;
    }

    switch (dbfID)
    {
        case 57559: // Smart Savings
            if (event == Season14HeroPowerBatch2Event::SELL_MINION)
            {
                state.deferredGoldNextTurn += 1;
            }
            else if (event == Season14HeroPowerBatch2Event::BEGIN_TURN)
            {
                result.goldDelta = state.deferredGoldNextTurn;
                state.deferredGoldNextTurn = 0;
            }
            break;
        case 63605: // Everbloom
            if (event == Season14HeroPowerBatch2Event::UPGRADE_TAVERN)
            {
                result.goldDelta = 2;
            }
            break;
        case 64476: // Avalanche
            if (event == Season14HeroPowerBatch2Event::PLAY_ELEMENTAL)
            {
                ++state.elementalPlays;
                if (state.elementalPlays >= 3)
                {
                    state.elementalPlays = 0;
                    state.upgradeCostReduction += 3;
                    result.upgradeCostDelta = -3;
                }
            }
            break;
        case 117426: // Arcane Knowledge
            if (event == Season14HeroPowerBatch2Event::BEGIN_TURN &&
                state.turnNumber == 3 && !state.arcaneKnowledgeUnlocked)
            {
                state.arcaneKnowledgeUnlocked = true;
                state.tavernSpellDiscount = 1;
                result.spellCostDelta = -1;
            }
            break;
        default:
            break;
    }
}

//! Resolve target-free active powers.  Targeted, random, and choice powers
//! intentionally return false until their offering/target contract exists.
constexpr bool ResolveSeason14HeroPowerBatch2Activation(
    std::int32_t dbfID, Season14HeroPowerBatch2State& state,
    bool lastTavernSpellAvailable,
    Season14HeroPowerBatch2Result& result) noexcept
{
    result = {};
    if (dbfID == 71459) // Bloodbound: twice per turn, +2 Blood Gems.
    {
        if (state.bloodboundUsesThisTurn >= 2)
        {
            return false;
        }
        ++state.bloodboundUsesThisTurn;
        result.bloodGemDelta = 2;
        return true;
    }
    if (dbfID == 58537) // Temporal Tavern: refresh and two higher-tier minions.
    {
        state.higherTierRefreshMinions = 2;
        result.extraHigherTierMinions = 2;
        return true;
    }
    if (dbfID == 116924) // The Galaxy's Lens requires a previous spell.
    {
        if (!lastTavernSpellAvailable)
        {
            return false;
        }
        result.copyLastTavernSpell = true;
        result.heroPowerCostDelta = -1;
        state.nextHeroPowerDiscount = true;
        return true;
    }
    return false;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HERO_POWER_BEHAVIORS_BATCH2_HPP
