#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>

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

TEST_CASE("[Season14] - Tavern spell modal retains source and branches")
{
    Season14State state;
    state.BeginSpellTargetChoice(12345, 2, 99, 3, 1, 1, 3,
                                 "friendly_minion_target");

    CHECK(state.pendingDecision == Season14Decision::CHOOSE_ONE);
    CHECK(state.spellModal.kind == Season14SpellModalKind::TARGET_STATS);
    CHECK(state.spellModal.sourceCardDbfID == 12345);
    CHECK(state.spellModal.targetIndex == 2);
    CHECK(state.spellModal.offeringFilter == "friendly_minion_target");

    int attack = 0;
    int health = 0;
    CHECK(!state.SelectSpellTargetChoice(2, attack, health));
    CHECK(state.spellModal.kind == Season14SpellModalKind::TARGET_STATS);
    CHECK(state.SelectSpellTargetChoice(0, attack, health));
    CHECK(attack == 3);
    CHECK(health == 1);
    CHECK(state.pendingDecision == Season14Decision::NONE);
    CHECK(state.spellModal.kind == Season14SpellModalKind::NONE);
    CHECK(!state.SelectSpellTargetChoice(1, attack, health));
}

TEST_CASE("[Season14] - concrete choice offerings resolve into hand")
{
    Player player;
    const auto minion = Cards::FindCardByDbfID(49169);
    REQUIRE(minion.GetCardType() == CardType::MINION);

    player.season14.BeginDecision(
        Season14Decision::DISCOVER,
        { Season14Offering{ minion.dbfID, 77 } });
    CHECK(player.ApplyChoice(0));
    CHECK(player.hand.GetCount() == 1);
    CHECK(player.season14.pendingDecision == Season14Decision::NONE);
    CHECK(std::get<Minion>(player.hand[0]).GetDbfID() == minion.dbfID);
}

TEST_CASE("[Season14] - unsupported choice remains pending")
{
    Player player;
    player.season14.BeginDecision(
        Season14Decision::CHOICE,
        { Season14Offering{ 999999, 123 } });
    CHECK(!player.ApplyChoice(0));
    CHECK(player.hand.GetCount() == 0);
    CHECK(player.season14.pendingDecision == Season14Decision::CHOICE);
    CHECK(player.season14.pendingOfferings.size() == 1);
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

TEST_CASE("[Season14] - Bob's Tip Jar grants immediate gold and raises cap")
{
    Season14State state;
    state.AddTrinket({112988, 1, true}); // BG30_MagicItem_996

    CHECK(state.TakeImmediateGold() == 4);
    CHECK(state.TakeImmediateGold() == 0);
    CHECK(state.EffectiveMaxGold(10) == 14);
}

TEST_CASE("[Season14] - Bob-blehead grants immediate gold without raising cap")
{
    Season14State state;
    state.AddTrinket({113101, 1, true}); // BG30_MagicItem_998

    CHECK(state.TakeImmediateGold() == 2);
    CHECK(state.EffectiveMaxGold(10) == 10);
}

TEST_CASE("[Season14] - Trinket slots reject duplicate and invalid acquisition")
{
    Player player;

    // BG30_MagicItem_996 is a known Trinket and occupies one slot exactly
    // once, even if an acquisition event is delivered twice.
    CHECK(player.AcquireTrinket({112988, 1, true}));
    CHECK(!player.AcquireTrinket({112988, 1, true}));
    CHECK(player.season14.trinkets.size() == 1);

    // Unknown/non-Trinket DBF ids must fail before persistent state changes.
    CHECK(!player.AcquireTrinket({999999, 1, true}));
    CHECK(player.season14.trinkets.size() == 1);
}

TEST_CASE("[Season14] - inactive start-turn Trinkets do not grant")
{
    Player player;
    player.season14.trinkets.push_back({113103, 1, false}); // Pagle's Fishing Rod
    CHECK(player.GrantTrinketStartTurnCards() == 0);
    CHECK(player.hand.GetCount() == 0);
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

TEST_CASE("[Season14] - next-combat reward is owner-relative and one-shot")
{
    Season14State state;
    state.ArmNextCombatReward(105267);
    state.ResolveNextCombatReward(BattleResult::PLAYER1_WIN, true);
    CHECK(state.TakeNextTurnGold() == 3);
    CHECK(state.TakeNextTurnGold() == 0);

    state.ArmNextCombatReward(105267);
    state.ResolveNextCombatReward(BattleResult::DRAW, false);
    CHECK(state.TakeNextTurnGold() == 1);

    state.ArmNextCombatReward(105267);
    state.ResolveNextCombatReward(BattleResult::PLAYER1_WIN, false);
    CHECK(state.TakeNextTurnGold() == 0);
}

TEST_CASE("[Season14] - spend-gold thresholds roll over and reset per recruit turn")
{
    Season14State state;
    CHECK(state.RecordGoldSpent(4) == 0);
    CHECK(state.RecordGoldSpent(1) == 1);
    CHECK(state.RecordGoldSpent(9) == 2);
    CHECK(state.RecordGoldSpent(1) == 0);
    CHECK(state.RecordGoldSpent(5) == 1);
    state.BeginRecruitTurn();
    CHECK(state.RecordGoldSpent(4) == 0);
    CHECK(state.RecordGoldSpent(1) == 1);
}

TEST_CASE("[Season14] - persistent race bonuses stack and remain state-owned")
{
    Season14State state;
    state.AddPersistentRaceStats(Race::UNDEAD, 1, 0);
    state.AddPersistentRaceStats(Race::UNDEAD, 2, 1);
    REQUIRE(state.persistentRaceStats.size() == 1);
    CHECK(state.persistentRaceStats.front().race == Race::UNDEAD);
    CHECK(state.persistentRaceStats.front().attack == 3);
    CHECK(state.persistentRaceStats.front().health == 1);
    state.BeginRecruitTurn();
    CHECK(state.persistentRaceStats.front().attack == 3);
}
