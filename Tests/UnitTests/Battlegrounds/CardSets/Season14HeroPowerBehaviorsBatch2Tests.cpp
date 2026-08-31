// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch2.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - exact unique registry")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH2.size() == 12);
    for (std::size_t i = 0; i < SEASON14_HERO_POWER_BEHAVIORS_BATCH2.size(); ++i)
    {
        const auto& entry = SEASON14_HERO_POWER_BEHAVIORS_BATCH2[i];
        REQUIRE(FindSeason14HeroPowerBehaviorBatch2(entry.id) != nullptr);
        REQUIRE(FindSeason14HeroPowerBehaviorBatch2(entry.dbfID) != nullptr);
        CHECK(FindSeason14HeroPowerBehaviorBatch2(entry.id)->dbfID == entry.dbfID);
        for (std::size_t j = i + 1;
             j < SEASON14_HERO_POWER_BEHAVIORS_BATCH2.size(); ++j)
        {
            CHECK(entry.id != SEASON14_HERO_POWER_BEHAVIORS_BATCH2[j].id);
            CHECK(entry.dbfID != SEASON14_HERO_POWER_BEHAVIORS_BATCH2[j].dbfID);
        }
    }
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - Growing Collection unlocks at turn eight")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};
    for (int i = 0; i < 7; ++i)
        ++state.turnNumber;
    CHECK(!ResolveSeason14HeroPowerBatch2Activation(120650, state, false, result));
    ++state.turnNumber;
    CHECK(ResolveSeason14HeroPowerBatch2Activation(120650, state, false, result));
    CHECK(result.beginGreaterTrinketOffer);
    CHECK(state.growingCollectionOffered);
    CHECK(!ResolveSeason14HeroPowerBatch2Activation(120650, state, false, result));
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - passive modifiers are explicit")
{
    const auto frost = Season14HeroPowerBatch2Modifiers(57945);
    CHECK(frost.minionCost == 2);
    CHECK(frost.refreshCost == 2);
    CHECK(frost.tavernSlotsDelta == -1);
    CHECK(frost.freezeRemainingShopAtEnd);

    const auto mana = Season14HeroPowerBatch2Modifiers(126538);
    CHECK(mana.refreshWithTavernSpells);

    const auto golden = Season14HeroPowerBatch2Modifiers(126533);
    CHECK(golden.twoCopiesMakeGolden);

    CHECK(Season14HeroPowerBatch2Modifiers(57559).minionCost == 0);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - every third Tavern spell is free")
{
    Season14State state;
    state.heroPowerDbfID = 105432;
    CHECK(state.TavernSpellCost(1) == 1);
    state.OnTavernSpellResolved(true);
    CHECK(state.TavernSpellCost(1) == 1);
    state.OnTavernSpellResolved(true);
    CHECK(state.TavernSpellCost(1) == 0);
    state.OnTavernSpellResolved(true);
    CHECK(state.TavernSpellCost(1) == 1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - lifecycle counters resolve")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};

    ResolveSeason14HeroPowerBatch2Event(
        57559, Season14HeroPowerBatch2Event::SELL_MINION, state, result);
    ResolveSeason14HeroPowerBatch2Event(
        57559, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(state.turnNumber == 1);
    CHECK(result.goldDelta == 1);
    CHECK(state.deferredGoldNextTurn == 0);

    ResolveSeason14HeroPowerBatch2Event(
        64476, Season14HeroPowerBatch2Event::PLAY_ELEMENTAL, state, result);
    ResolveSeason14HeroPowerBatch2Event(
        64476, Season14HeroPowerBatch2Event::PLAY_ELEMENTAL, state, result);
    ResolveSeason14HeroPowerBatch2Event(
        64476, Season14HeroPowerBatch2Event::PLAY_ELEMENTAL, state, result);
    CHECK(result.upgradeCostDelta == -3);
    CHECK(state.upgradeCostReduction == 3);

    ResolveSeason14HeroPowerBatch2Event(
        63605, Season14HeroPowerBatch2Event::UPGRADE_TAVERN, state, result);
    CHECK(result.goldDelta == 2);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - activations enforce lifecycle")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};

    CHECK(ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                   result));
    CHECK(result.bloodGemDelta == 2);
    CHECK(ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                   result));
    CHECK(result.bloodGemDelta == 2);
    CHECK(!ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                    result));

    CHECK(ResolveSeason14HeroPowerBatch2Activation(58537, state, false,
                                                   result));
    CHECK(result.extraHigherTierMinions == 2);
    CHECK(state.higherTierRefreshMinions == 2);

    CHECK(!ResolveSeason14HeroPowerBatch2Activation(116924, state, false,
                                                    result));
    CHECK(ResolveSeason14HeroPowerBatch2Activation(116924, state, true,
                                                    result));
    CHECK(result.copyLastTavernSpell);
    CHECK(result.heroPowerCostDelta == -1);
    CHECK(state.nextHeroPowerDiscount);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - Bloodbound is exactly two uses and resets")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};

    CHECK(ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                   result));
    CHECK(result.bloodGemDelta == 2);
    CHECK(state.bloodboundUsesThisTurn == 1);
    CHECK(ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                   result));
    CHECK(result.bloodGemDelta == 2);
    CHECK(state.bloodboundUsesThisTurn == 2);
    CHECK(!ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                    result));

    ResolveSeason14HeroPowerBatch2Event(
        71459, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(state.turnNumber == 1);
    CHECK(state.bloodboundUsesThisTurn == 0);
    CHECK(ResolveSeason14HeroPowerBatch2Activation(71459, state, false,
                                                   result));
    CHECK(result.bloodGemDelta == 2);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - Arcane Knowledge unlocks exactly on turn three")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};

    ResolveSeason14HeroPowerBatch2Event(
        117426, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(result.spellCostDelta == 0);
    ResolveSeason14HeroPowerBatch2Event(
        117426, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(result.spellCostDelta == 0);
    ResolveSeason14HeroPowerBatch2Event(
        117426, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(state.turnNumber == 3);
    CHECK(result.spellCostDelta == -1);
    CHECK(state.arcaneKnowledgeUnlocked);
    CHECK(state.tavernSpellDiscount == 1);

    ResolveSeason14HeroPowerBatch2Event(
        117426, Season14HeroPowerBatch2Event::BEGIN_TURN, state, result);
    CHECK(state.turnNumber == 4);
    CHECK(result.spellCostDelta == 0);
    CHECK(state.tavernSpellDiscount == 1);
}

TEST_CASE("[Season14HeroPowerBehaviorsBatch2] - Arcane Knowledge discount is consumed once on success")
{
    Season14HeroPowerBatch2State state{};
    Season14HeroPowerBatch2Result result{};

    for (int turn = 0; turn < 3; ++turn)
    {
        ResolveSeason14HeroPowerBatch2Event(
            117426, Season14HeroPowerBatch2Event::BEGIN_TURN, state,
            result);
    }

    CHECK(state.TavernSpellCost(2) == 1);
    CHECK(state.TavernSpellCost(0) == 0);

    // A failed or unsupported attempt does not consume the discount.
    CHECK(!state.ConsumeTavernSpellDiscount(false));
    CHECK(state.tavernSpellDiscount == 1);
    CHECK(state.TavernSpellCost(2) == 1);

    CHECK(state.ConsumeTavernSpellDiscount(true));
    CHECK(state.tavernSpellDiscount == 0);
    CHECK(state.TavernSpellCost(2) == 2);
    CHECK(!state.ConsumeTavernSpellDiscount(true));
}
