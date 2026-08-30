#include <Rosetta/Battlegrounds/Models/Season14.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14] - Public decisions validate and clear")
{
    Season14State state;
    state.BeginDecision(
        Season14Decision::DISCOVER,
        {Season14Offering{101, 1}, Season14Offering{102, 2}});

    CHECK(state.pendingDecision == Season14Decision::DISCOVER);
    CHECK(state.choiceOfferings.size() == 2);
    CHECK(!state.SelectDecision(2));
    CHECK(state.pendingDecision == Season14Decision::DISCOVER);
    CHECK(state.SelectDecision(1));
    CHECK(state.pendingDecision == Season14Decision::NONE);
    CHECK(state.pendingOfferings.empty());
}

TEST_CASE("[Season14] - Hero power availability is costed and one-shot")
{
    Season14State state;
    state.SetHeroPower(9001, 2, true);

    CHECK(!state.CanUseHeroPower(1));
    CHECK(state.CanUseHeroPower(2));
    CHECK(state.UseHeroPower());
    CHECK(!state.CanUseHeroPower(20));
    CHECK(!state.UseHeroPower());
}

TEST_CASE("[Season14] - Persistent effects and event hooks")
{
    Season14State state;
    state.AddTrinket({7001, 2, true});
    state.AddDarkGift({8001, 1, true});

    CHECK(state.ConsumeEffect(state.trinkets, 0));
    CHECK(state.trinkets[0].remainingUses == 1);
    CHECK(state.ConsumeEffect(state.trinkets, 0));
    CHECK(!state.trinkets[0].active);
    CHECK(!state.ConsumeEffect(state.trinkets, 0));
    CHECK(state.ConsumeEffect(state.darkGifts, 0));
    CHECK(!state.darkGifts[0].active);

    // Acquisitions must carry a usable charge; an active zero-use entry must
    // never become an unlimited effect through the consume path.
    state.AddDarkGift({8002, 0, true});
    CHECK(state.darkGifts.size() == 1);
    state.darkGifts[0].remainingUses = 0;
    state.darkGifts[0].active = true;
    CHECK(!state.ConsumeEffect(state.darkGifts, 0));

    state.Emit(Season14Event::RECRUIT_START);
    state.Emit(Season14Event::COMBAT_END);
    CHECK(state.eventCounts[0] == 1);
    CHECK(state.eventCounts[3] == 1);
    CHECK(Season14State::IsValidBoardTarget(0, 1));
    CHECK(!Season14State::IsValidBoardTarget(1, 1));
}

TEST_CASE("[Season14] - selected hero installs deterministic lifecycle hooks")
{
    Season14State state;

    state.SetHeroPower(59399, 0, true);
    CHECK(state.heroPowerBatch1.StartingHealth(40) == 70);
    CHECK(state.heroPowerBatch1.StartingHealth(60) == 90);

    state.SetHeroPower(60405, 0, true);
    CHECK(state.MinionPurchaseCost(3) == 5);
    CHECK(state.RefreshCost(1) == 3);
    CHECK(state.UpgradeCost(5) == 6);

    state.SetHeroPower(57945, 0, true);
    CHECK(state.TavernOfferCount(3) == 2);
    CHECK(state.ShouldFreezeRemainingTavern());

    state.SetHeroPower(122960, 0, true);
    CHECK(state.TavernSpellCost(3) == 3);

    state.SetHeroPower(61491, 0, true);
    state.BeginRecruitTurn();
    CHECK(state.RefreshCost(1) == 0);
    state.OnRefreshTavern(true);
    CHECK(state.RefreshCost(1) == 1);
    state.OnRefreshTavern(false);
    CHECK(state.RefreshCost(1) == 1);
}

TEST_CASE("[Season14] - lifecycle hooks pay deterministic Batch-2 effects")
{
    Season14State state;
    state.SetHeroPower(57559, 1, true);

    state.OnSellMinion();
    const auto result = state.BeginRecruitTurn();
    CHECK(result.goldDelta == 1);
    CHECK(state.heroPowerBatch2.turnNumber == 1);

    state.SetHeroPower(117426, 0, true);
    state.BeginRecruitTurn();
    state.BeginRecruitTurn();
    state.BeginRecruitTurn();
    CHECK(state.TavernSpellCost(2) == 1);
    state.OnTavernSpellResolved(false);
    CHECK(state.TavernSpellCost(2) == 1);
    state.OnTavernSpellResolved(true);
    CHECK(state.TavernSpellCost(2) == 2);
}

TEST_CASE("[Season14] - hero-power discount is consumed by successful use")
{
    Season14State state;
    state.SetHeroPower(116924, 3, true);
    state.heroPowerBatch2.nextHeroPowerDiscount = true;

    CHECK(state.EffectiveHeroPowerCost() == 2);
    CHECK(state.CanUseHeroPower(2));
    CHECK(state.UseHeroPower());
    CHECK(!state.heroPowerBatch2.nextHeroPowerDiscount);
    CHECK(!state.CanUseHeroPower(3));
}

TEST_CASE("[Season14] - Temporal Tavern refresh allowance is one-shot")
{
    Season14State state;
    state.SetHeroPower(58537, 1, true);
    state.ArmHigherTierRefresh(2);
    CHECK(state.TakeHigherTierRefresh() == 2);
    CHECK(state.TakeHigherTierRefresh() == 0);
}

TEST_CASE("[Season14] - simple Tavern spell economy state is deterministic")
{
    Season14State state;

    state.AddNextTurnGold(2);
    state.AddNextTurnGold(1);
    CHECK(state.TakeNextTurnGold() == 3);
    CHECK(state.TakeNextTurnGold() == 0);

    CHECK(state.EffectiveMaxGold(10) == 10);
    state.IncreaseMaxGold(1);
    CHECK(state.EffectiveMaxGold(10) == 11);

    state.AddFreeRefreshes(2);
    CHECK(state.HasFreeRefresh());
    CHECK(state.ConsumeFreeRefresh());
    CHECK(state.ConsumeFreeRefresh());
    CHECK(!state.HasFreeRefresh());
    CHECK(!state.ConsumeFreeRefresh());

    state.AddPersistentShopStats(2, 2);
    CHECK(state.persistentShopAttack == 2);
    CHECK(state.persistentShopHealth == 2);
}
