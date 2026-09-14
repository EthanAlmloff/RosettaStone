#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MaxHealthDeathrattleTask.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14] - Bloodfury one-shot Fodders stack separately from refresh windows")
{
    Season14State state;
    state.ArmFodderRefreshes(3, 1);
    state.ArmFoddersNextRefresh(2);

    CHECK(state.ConsumeFodderRefresh() == 1);
    CHECK(state.ConsumeFoddersNextRefresh() == 2);
    CHECK(state.ConsumeFoddersNextRefresh() == 0);
    CHECK(state.ConsumeFodderRefresh() == 1);
    CHECK(state.ConsumeFodderRefresh() == 1);
    CHECK(state.ConsumeFodderRefresh() == 0);

    state.ArmFoddersNextRefresh(99);
    CHECK(state.ConsumeFoddersNextRefresh() == MAX_FIELD_SIZE);
}

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

TEST_CASE("[Season14] - Perfect Crime cost discounts once per recruit turn")
{
    const auto* definition = FindSeason14HeroPowerBehavior("BG23_HERO_305p");
    REQUIRE(definition != nullptr);
    CHECK(definition->kind == Season14HeroPowerKind::PERFECT_CRIME);
    CHECK(definition->cost == 11);
    Season14State state;
    state.SetHeroPower(86292, 11, true);
    CHECK(state.EffectiveHeroPowerCost() == 11);
    state.BeginRecruitTurn();
    CHECK(state.EffectiveHeroPowerCost() == 10);
    state.BeginRecruitTurn();
    CHECK(state.EffectiveHeroPowerCost() == 9);
    state.BeginRecruitTurn();
    CHECK(state.EffectiveHeroPowerCost() == 8);
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

TEST_CASE("[Season14] - Buddy extra hero-power use is consumable")
{
    Season14State state;
    state.SetHeroPower(70957, 2, true);

    CHECK(state.UseHeroPower());
    state.buddyExtraHeroPowerUses = 1;
    CHECK(state.CanUseHeroPower(2));
    CHECK(state.UseHeroPower());
    CHECK(state.buddyExtraHeroPowerUses == 0);
    CHECK(!state.CanUseHeroPower(2));
    CHECK(!state.UseHeroPower());
}

TEST_CASE("[Season14] - Double Time turns two copies into a golden and Tavern Coin")
{
    Player player;
    player.season14.SetHeroPower(126533, 0, true);
    const auto card = Cards::FindCardByDbfID(49169);
    REQUIRE(card.GetCardType() == CardType::MINION);
    REQUIRE(card.premiumDbfID != 0);
    player.hand.Add(CardData{Minion(card)});
    player.hand.Add(CardData{Minion(card)});

    CHECK(player.ResolveDoubleTimeCopies());
    CHECK(player.hand.GetCount() == 2);
    CHECK(std::holds_alternative<Minion>(player.hand[0]));
    CHECK(std::get<Minion>(player.hand[0]).IsGolden());
    REQUIRE(std::holds_alternative<Spell>(player.hand[1]));
    CHECK(std::get<Spell>(player.hand[1]).GetCardID() == "BG28_810");
    CHECK(!player.ResolveDoubleTimeCopies());
}

TEST_CASE("[Season14] - Designer Eyepatch pairs only Pirates, including a full hand")
{
    Player player;
    player.season14.trinkets.push_back({Cards::FindCardByID("BG30_MagicItem_439").dbfID,
                                        1, true});
    const auto pirate = Cards::FindCardByID("BG21_017");
    const auto nonPirate = Cards::FindCardByDbfID(49169);
    REQUIRE(pirate.premiumDbfID != 0);
    REQUIRE(nonPirate.premiumDbfID != 0);

    // A full hand cannot prevent an in-place board conversion.  The normal
    // Triple path leaves the unmatched third copy untouched.
    for (int i = 0; i < MAX_HAND_SIZE; ++i)
        player.hand.Add(CardData{Minion(nonPirate)});
    player.recruitField.Add(Minion(pirate));
    player.recruitField.Add(Minion(pirate));
    player.recruitField.Add(Minion(nonPirate));

    CHECK(player.ResolveDoubleTimeCopies());
    CHECK(player.recruitField.GetCount() == 2);
    CHECK(player.recruitField[0].IsGolden());
    CHECK(player.recruitField[0].HasRace(Race::PIRATE));
    CHECK(!player.recruitField[1].IsGolden());
    CHECK(player.hand.GetCount() == MAX_HAND_SIZE);
}

TEST_CASE("[Season14] - Designer Eyepatch merges both copy states without a coin")
{
    const auto pirate = Cards::FindCardByID("BG21_017");
    REQUIRE(pirate.premiumDbfID != 0);
    const auto golden = Cards::FindCardByDbfID(pirate.premiumDbfID);
    Minion survivor(pirate);
    Minion duplicate(pirate);

    survivor.ApplyPersistentMinionStats(2, 3);
    duplicate.ApplyPersistentMinionStats(4, 5);
    survivor.SetTaunt(true);
    duplicate.SetGameTag(GameTag::POISONOUS, 1);
    survivor.AddDarkGiftDeathrattleTask(
        SimpleTasks::MaxHealthDeathrattleTask{1});
    duplicate.AddDarkGiftDeathrattleTask(
        SimpleTasks::MaxHealthDeathrattleTask{2});
    survivor.RecordTemporaryEnchantment("EYEPATCH_FIRST");
    duplicate.RecordTemporaryEnchantment("EYEPATCH_SECOND");

    REQUIRE(survivor.MergeIntoGolden(duplicate));
    CHECK(survivor.IsGolden());
    CHECK(survivor.GetAttack() == golden.GetAttack() + 6);
    CHECK(survivor.GetHealth() == golden.GetHealth() + 8);
    CHECK(survivor.HasTaunt());
    CHECK(survivor.HasVenomous());
    CHECK(survivor.HasTemporaryEnchantment("EYEPATCH_FIRST"));
    CHECK(survivor.HasTemporaryEnchantment("EYEPATCH_SECOND"));
    CHECK(survivor.GetTasks(PowerType::DEATHRATTLE).size() == 2);
}

TEST_CASE("[Season14] - Eyepatch preserves the board copy in a hand-board pair")
{
    Player player;
    player.season14.trinkets.push_back({Cards::FindCardByID("BG30_MagicItem_439").dbfID,
                                        1, true});
    const auto pirate = Cards::FindCardByID("BG21_017");
    REQUIRE(pirate.premiumDbfID != 0);

    Minion handCopy(pirate);
    Minion boardCopy(pirate);
    handCopy.ApplyPersistentMinionStats(1, 2);
    boardCopy.ApplyPersistentMinionStats(4, 5);
    boardCopy.AddDarkGiftDeathrattleTask(
        SimpleTasks::MaxHealthDeathrattleTask{3});
    player.hand.Add(CardData{std::move(handCopy)});
    player.recruitField.Add(boardCopy);

    REQUIRE(player.ResolveDoubleTimeCopies());
    REQUIRE(player.hand.GetCount() == 1);
    REQUIRE(std::holds_alternative<Minion>(player.hand[0]));
    const auto& result = std::get<Minion>(player.hand[0]);
    CHECK(result.IsGolden());
    CHECK(result.GetAttack() == Cards::FindCardByDbfID(pirate.premiumDbfID).GetAttack() + 5);
    CHECK(result.GetHealth() == Cards::FindCardByDbfID(pirate.premiumDbfID).GetHealth() + 7);
    CHECK(result.GetTasks(PowerType::DEATHRATTLE).size() == 1);
    CHECK(player.recruitField.GetCount() == 0);
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

TEST_CASE("[Season14] - Mysterious Orb pays Gold at acquisition")
{
    Player player;
    player.remainCoin = 3;

    // BG35_MagicItem_818 is DBF 130836.  Its ten Gold is spendable in the
    // current recruit phase and is not queued for a later Recruit boundary.
    CHECK(player.AcquireTrinket({130836, 1, true}));
    CHECK(player.remainCoin == 13);
    CHECK(player.season14.TakeImmediateGold() == 0);
    CHECK(player.season14.mysteriousOrbLesserNext);
}

TEST_CASE("[Season14] - inactive start-turn Trinkets do not grant")
{
    Player player;
    player.season14.trinkets.push_back({113103, 1, false}); // Pagle's Fishing Rod
    CHECK(player.GrantTrinketStartTurnCards() == 0);
    CHECK(player.hand.GetCount() == 0);
}

TEST_CASE("[Season14] - acquisition grants recurring Trinkets exactly once")
{
    Player player;
    // Essence of Dreams grants two Dreamer's Embrace spells on acquisition;
    // the recruit-start cadence is one additional spell.  This guards
    // against dispatching the immediate grant twice.
    CHECK(player.AcquireTrinket({111253, 1, true})); // BG30_MagicItem_916
    CHECK(player.hand.GetCount() == 2);
    CHECK(player.GrantTrinketStartTurnCards() == 1);
    CHECK(player.hand.GetCount() == 3);
}

TEST_CASE("[Season14] - golden Egg portrait arms the pinned next-turn countdown")
{
    Player player;
    // BG35_MagicItem_848t is Egg of the Endtimes Portrait (DBF 130900).
    CHECK(player.AcquireTrinket({130900, 1, true}));
    REQUIRE(player.hand.GetCount() == 1);
    REQUIRE(std::holds_alternative<Minion>(player.hand[0]));
    const auto& egg = std::get<Minion>(player.hand[0]);
    CHECK(egg.GetCardID() == "BG34_639_G");
    CHECK(egg.EggHatchTurnsRemaining() == 1);
}

TEST_CASE("[Season14] - consumed Egg Discover remains executable")
{
    Player player;
    const auto eggCard = Cards::FindCardByID("BG34_639");
    REQUIRE(eggCard.dbfID == 126848);
    Minion egg(eggCard);
    egg.SetEggHatch(1);
    player.hand.Add(CardData{std::move(egg)});

    player.ResolveDarkGiftEndTurnTriggers();

    CHECK(player.hand.GetCount() == 0);
    CHECK(player.season14.pendingDecision == Season14Decision::DISCOVER);
    CHECK(player.season14.pendingSourceCardDbfID == 126848);
    CHECK(player.season14.pendingSourceEntityID == 0);
    REQUIRE(!player.season14.pendingOfferings.empty());
    CHECK(player.ApplyChoice(0));
    CHECK(player.hand.GetCount() == 1);
    CHECK(player.season14.pendingDecision == Season14Decision::NONE);
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

    // Skilled Bartender (DBF 57561) is a passive Batch8 power.  Its
    // executable ownership is the Season14 upgrade-cost boundary, not an
    // activation branch, so keep an explicit regression check here.
    state.SetHeroPower(57561, 0, true);
    CHECK(state.UpgradeCost(5) == 4);

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

    // Demon Hunter Training targets seven offers after five refreshes; it is
    // not a fixed two-slot bonus (which would leave low-tier Taverns short).
    state.SetHeroPower(61915, 0, true);
    state.heroPowerBatch5.demonHunterTrainingUnlocked = true;
    CHECK(state.TavernOfferCount(3) == 7);
    CHECK(state.TavernOfferCount(6) == 7);
    CHECK(state.TavernOfferCount(7) == 7);
}

TEST_CASE("[Season14] - Felbat and absolute Tavern portraits target seven offers")
{
    Season14State state;
    // Felbat Portrait (DBF 112054) grants Famished Felbat and sets a minimum
    // Tavern size; the target must apply below tier 4 and remain seven above it.
    state.AddTrinket({112054, 1, true});
    CHECK(state.TavernOfferCount(3) == 7);
    CHECK(state.TavernOfferCount(6) == 7);
    CHECK(state.TavernOfferCount(7) == 7);

    // A second absolute-seven portrait must not turn the target into a flat
    // +1 modifier.  A genuine extra-slot modifier remains additive, however.
    state.AddTrinket({111092, 1, true});
    CHECK(state.TavernOfferCount(3) == 7);
    CHECK(state.TavernOfferCount(6) == 7);
}

TEST_CASE("[Season14] - Electrode Attractor owns magnetic discount and refresh offer")
{
    Season14State state;
    state.AddTrinket({131141, 1, true}); // BG35_MagicItem_743
    CHECK(state.MagneticMechPurchaseCostDiscount() == 2);
    CHECK(state.HasMagneticMechFixedCost());
    CHECK(state.TavernOfferCount(3) == 3); // no bonus before a refresh
    state.refreshExtraShopSlots = 1;
    CHECK(state.TavernOfferCount(3) == 4);
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

TEST_CASE("[Season14] - spell-count Trinkets use shared resolved-spell callback")
{
    Season14State state;
    state.AddTrinket({120610, 1, true}); // BG32_MagicItem_930
    state.AddTrinket({133379, 1, true}); // BG36_MagicItem_307

    for (int i = 0; i < 6; ++i)
        state.OnTavernSpellResolved(true, 0, false);
    CHECK(state.PendingSpellCountNagaRewards() == 0);
    CHECK(state.TakeSpellCountGold() == 0);

    // The seventh spell produces Archaic Scroll's Naga reward; only spells
    // cast on minions advance Wand of Divination.
    state.OnTavernSpellResolved(true, 0, false);
    CHECK(state.PendingSpellCountNagaRewards() == 1);
    CHECK(state.TakeSpellCountGold() == 0);
    for (int i = 0; i < 2; ++i)
        state.OnTavernSpellResolved(true, 0, true);
    CHECK(state.TakeSpellCountGold() == 0);
    state.OnTavernSpellResolved(true, 0, true);
    CHECK(state.TakeSpellCountGold() == 1);
    // Both counters reset at their own thresholds and can trigger again.
    for (int i = 0; i < 7; ++i)
        state.OnTavernSpellResolved(true, 0, false);
    CHECK(state.PendingSpellCountNagaRewards() == 2);
}

TEST_CASE("[Season14] - Bubble Crown improves Tavern spell stats once")
{
    Season14State state;
    const auto crown = Cards::FindCardByID("BG35_MagicItem_920").dbfID;
    state.trinkets.push_back({crown, 1, true});

    CHECK(state.tavernSpellAttackBonus == 0);
    CHECK(state.tavernSpellHealthBonus == 0);
    for (int i = 0; i < 11; ++i)
        state.OnTavernSpellResolved(true);
    CHECK(state.tavernSpellAttackBonus == 0);
    CHECK(state.tavernSpellHealthBonus == 0);

    state.OnTavernSpellResolved(true);
    CHECK(state.tavernSpellAttackBonus == 4);
    CHECK(state.tavernSpellHealthBonus == 4);
    state.OnTavernSpellResolved(true);
    CHECK(state.tavernSpellAttackBonus == 4);
    CHECK(state.tavernSpellHealthBonus == 4);
}

TEST_CASE("[Season14] - spell-count Trinkets are per-instance and retry hand rewards")
{
    // Two Archaic Scrolls have independent cadence; an inactive copy must not
    // advance, and a full hand must leave the generated Naga reward pending.
    Player player;
    const auto scroll = Cards::FindCardByID("BG32_MagicItem_930").dbfID;
    player.season14.trinkets.push_back({scroll, 1, true});
    player.season14.trinkets.push_back({scroll, 1, true});
    player.season14.trinkets.push_back({scroll, 1, false});
    for (int i = 0; i < 7; ++i)
        player.season14.OnTavernSpellResolved(true, 0, false);
    CHECK(player.season14.PendingSpellCountNagaRewards() == 2);

    const auto minion = Cards::FindCardByDbfID(49169);
    for (int i = 0; i < MAX_HAND_SIZE; ++i)
        player.hand.Add(CardData{Minion(minion)});
    player.ResolveSpellCountTrinkets();
    CHECK(player.hand.GetCount() == MAX_HAND_SIZE);
    CHECK(player.season14.PendingSpellCountNagaRewards() == 2);

    auto held = player.hand[0];
    player.hand.Remove(held);
    player.ResolveSpellCountTrinkets();
    CHECK(player.hand.GetCount() == MAX_HAND_SIZE);
    CHECK(player.season14.PendingSpellCountNagaRewards() == 1);
}

TEST_CASE("[Season14] - sell and death random Trinkets keep independent cadence")
{
    const std::array<std::pair<const char*, int>, 5> cases{{
        {"BG30_MagicItem_710", 5},  // Fungalmancer Sticker, Murloc
        {"BG30_MagicItem_951", 6},  // Lava Lamp, Elemental
        {"BG30_MagicItem_713", 8},  // Bleeding Heart, Undead
        {"BG30_MagicItem_931", 7},  // Lucky Tabby, Beast
        {"BG35_MagicItem_302", 8},  // Stormcoil Sticker, Mech
    }};
    const auto minion = Cards::FindCardByDbfID(49169);
    for (const auto& [id, threshold] : cases)
    {
        Player player;
        const auto dbfID = Cards::FindCardByID(id).dbfID;
        REQUIRE(dbfID != 0);
        player.season14.trinkets.push_back({dbfID, 1, true});
        for (int i = 0; i < threshold * 2; ++i)
        {
            player.recruitField.Add(Minion(minion));
            player.SellMinion(0);
        }
        // The counter resets after delivery, so two complete cadences produce
        // two cards; race filtering is exercised by the production task.
        CHECK(player.hand.GetCount() == 2);
        CHECK(player.season14.trinkets.front().triggerProgress == 0);
    }
}

TEST_CASE("[Season14] - Cloud Serpent Horn keeps multiple triggered copies")
{
    Player player;
    const auto dbfID = Cards::FindCardByID("BG35_MagicItem_849").dbfID;
    REQUIRE(dbfID != 0);
    player.season14.trinkets.push_back({dbfID, 1, true});
    player.season14.trinkets.push_back({dbfID, 1, true});

    CHECK(player.season14.OnTrinketFriendlyMinionDied()
              .transferRightmostAttackToDragon == 0);
    CHECK(player.season14.OnTrinketFriendlyMinionDied()
              .transferRightmostAttackToDragon == 0);
    const auto result = player.season14.OnTrinketFriendlyMinionDied();
    CHECK(result.transferRightmostAttackToDragon == 2);
    CHECK(player.season14.trinkets[0].triggerProgress == 0);
    CHECK(player.season14.trinkets[1].triggerProgress == 0);
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

TEST_CASE("[Season14] - Reborn trinket is a friendly-board-only trigger")
{
    const auto behavior = FindTrinketBehavior("BG36_MagicItem_205");
    CHECK(behavior.effect == TrinketEffect::AFTER_REBORN_STATS);
    CHECK(behavior.attack == 2);
    CHECK(behavior.health == 2);

    Player player;
    player.season14.trinkets.push_back({
        Cards::FindCardByID("BG36_MagicItem_205").dbfID, 1, true});
    player.season14.trinkets.push_back({
        Cards::FindCardByID("BG36_MagicItem_205").dbfID, 1, true});
    Minion minion(Cards::FindCardByDbfID(49169));
    player.battleField.Add(minion);
    player.isInCombat = true;
    const int attack = player.battleField[0].GetAttack();
    const int health = player.battleField[0].GetHealth();
    player.ApplyAfterRebornTrinkets();
    CHECK(player.battleField[0].GetAttack() == attack + 4);
    CHECK(player.battleField[0].GetHealth() == health + 4);
    CHECK(player.recruitField.GetCount() == 0);
}

TEST_CASE("[Season14] - Funeral Wreath copies each Reborn minion up to three times")
{
    const auto behavior = FindTrinketBehavior("BG36_MagicItem_217");
    CHECK(behavior.effect == TrinketEffect::AFTER_REBORN_COPY);
    CHECK(behavior.value == 3);

    Player player;
    player.season14.trinkets.push_back({
        Cards::FindCardByID("BG36_MagicItem_217").dbfID, 1, true});
    Minion minion(Cards::FindCardByDbfID(49169));
    player.ApplyAfterRebornTrinkets(&minion);
    player.ApplyAfterRebornTrinkets(&minion);
    player.ApplyAfterRebornTrinkets(&minion);
    player.ApplyAfterRebornTrinkets(&minion);
    CHECK(player.hand.GetCount() == 3);
    CHECK(player.season14.trinkets.front().triggerProgress == 3);
}

TEST_CASE("[Season14] - Funeral Wreath retries a full hand and resets per combat")
{
    const auto dbf = Cards::FindCardByID("BG36_MagicItem_217").dbfID;
    Player player;
    player.season14.trinkets.push_back({dbf, 1, true});
    Minion source(Cards::FindCardByDbfID(49169));

    // A failed add does not consume one of the three Reborn triggers.  Once
    // space opens, the same combat event allowance remains available.
    for (int i = 0; i < MAX_HAND_SIZE; ++i)
        player.hand.Add(CardData{Minion(Cards::FindCardByDbfID(49169))});
    player.ApplyAfterRebornTrinkets(&source);
    CHECK(player.hand.GetCount() == MAX_HAND_SIZE);
    CHECK(player.season14.trinkets.front().triggerProgress == 0);

    CardData removed = player.hand[0];
    player.hand.Remove(removed);
    player.ApplyAfterRebornTrinkets(&source);
    CHECK(player.hand.GetCount() == MAX_HAND_SIZE);
    CHECK(player.season14.trinkets.front().triggerProgress == 1);

    // The allowance is combat-local and must be restored for the next
    // combat, while remaining attached to this Trinket instance.
    player.season14.ResetTrinketAvengeProgress();
    CHECK(player.season14.trinkets.front().triggerProgress == 0);
    player.ApplyAfterRebornTrinkets(&source);
    CHECK(player.season14.trinkets.front().triggerProgress == 1);
}

TEST_CASE("[Season14] - Deathtouch Apple re-arms combat Reborn persistently")
{
    const auto dbf = Cards::FindCardByID("BG35_MagicItem_731").dbfID;
    Player player;
    player.season14.trinkets.push_back({dbf, 1, true});

    Minion recruit(Cards::FindCardByDbfID(49169));
    Minion combat = recruit;
    combat.ReviveWithReborn(); // the observed Reborn event has consumed it
    player.ApplyAfterRebornTrinkets(&combat);

    CHECK(combat.HasReborn());
    CHECK(player.season14.trinkets.front().triggerProgress == 1);

    // Combat copies are reconciled after the fight; the re-arm must survive
    // into the next recruit phase rather than living only on the copy.
    recruit.ReconcileCombatPersistentState(combat);
    CHECK(recruit.HasReborn());
}

TEST_CASE("[Season14] - Deathtouch Apple copies have independent turn caps")
{
    const auto dbf = Cards::FindCardByID("BG35_MagicItem_731").dbfID;
    Player player;
    player.season14.trinkets.push_back({dbf, 1, true});
    player.season14.trinkets.push_back({dbf, 1, true});
    Minion first(Cards::FindCardByDbfID(49169));
    Minion second(Cards::FindCardByDbfID(49169));

    player.ApplyAfterRebornTrinkets(&first);
    player.ApplyAfterRebornTrinkets(&second);
    CHECK(player.season14.trinkets[0].triggerProgress == 1);
    CHECK(player.season14.trinkets[1].triggerProgress == 1);

    player.season14.trinkets[0].triggerProgress = 3;
    player.season14.trinkets[1].triggerProgress = 3;
    player.season14.ResetTrinketAvengeProgress();
    CHECK(player.season14.trinkets[0].triggerProgress == 3);
    CHECK(player.season14.trinkets[1].triggerProgress == 3);
    player.ResolveStartTurnTrinkets();
    CHECK(player.season14.trinkets[0].triggerProgress == 0);
    CHECK(player.season14.trinkets[1].triggerProgress == 0);
}

TEST_CASE("[Season14] - Boom Controller trigger resets each combat")
{
    const auto dbf = Cards::FindCardByID("BG30_MagicItem_440").dbfID;
    Player player;
    player.season14.trinkets.push_back({dbf, 1, true});
    auto& trinket = player.season14.trinkets.front();
    trinket.triggerProgress = 1; // consumed by the previous combat

    player.season14.ResetTrinketAvengeProgress();

    CHECK(FindTrinketBehavior("BG30_MagicItem_440").effect ==
          TrinketEffect::BOOM_CONTROLLER_FIRST_MECH_COPY);
    CHECK(trinket.triggerProgress == 0);
}

TEST_CASE("[Season14] - Dragon's Eye respects active and consumed state")
{
    Player player;
    const auto dbf = Cards::FindCardByID("BG36_MagicItem_215").dbfID;
    player.season14.trinkets.push_back({dbf, 1, true});
    CHECK(player.ShouldDuplicateDragonBattlecry());
    player.season14.trinkets.front().remainingUses = 0;
    CHECK(!player.ShouldDuplicateDragonBattlecry());
    player.season14.trinkets.front().remainingUses = 1;
    player.season14.trinkets.front().active = false;
    CHECK(!player.ShouldDuplicateDragonBattlecry());
}

TEST_CASE("[Season14] - first-minion shield is once per recruit turn")
{
    Player player;
    const auto dbf = Cards::FindCardByID("BG36_MagicItem_811").dbfID;
    player.season14.trinkets.push_back({dbf, 1, true});
    Minion first(Cards::FindCardByDbfID(49169));
    Minion second(Cards::FindCardByDbfID(49169));
    player.recruitField.Add(first);
    player.ApplyFirstMinionDivineShield(player.recruitField[0]);
    CHECK(player.recruitField[0].HasDivineShield());
    player.recruitField.Add(second);
    player.ApplyFirstMinionDivineShield(player.recruitField[1]);
    CHECK(!player.recruitField[1].HasDivineShield());
    player.season14.firstMinionPlayedThisTurn = false;
    player.season14.trinkets.front().active = false;
    Minion third(Cards::FindCardByDbfID(49169));
    player.recruitField.Add(third);
    player.ApplyFirstMinionDivineShield(player.recruitField[2]);
    CHECK(!player.recruitField[2].HasDivineShield());
}

TEST_CASE("[Season14] - next-combat buff resolves only on owner win")
{
    Season14State state;
    state.ArmNextCombatBuff(133369, 42, 4, 6);
    state.ArmNextCombatBuff(133369, 42, 4, 6);
    std::vector<Season14PendingCombatBuff> resolved;
    CHECK(state.ResolveNextCombatBuff(BattleResult::PLAYER1_WIN, true,
                                      resolved));
    CHECK(resolved.size() == 2);
    CHECK(resolved.front().targetEntityID == 42);
    CHECK(resolved.front().attack == 4);

    state.ArmNextCombatBuff(133369, 42, 4, 6);
    CHECK(!state.ResolveNextCombatBuff(BattleResult::DRAW, true, resolved));
    CHECK(resolved.size() == 1);
    CHECK(!state.ResolveNextCombatBuff(BattleResult::PLAYER1_WIN, true,
                                       resolved));
}

TEST_CASE("[Season14] - combat-start attack doubles stack and reset")
{
    Season14State state;
    state.ArmCombatStartLeftmostAttackDouble(127503);
    state.ArmCombatStartLeftmostAttackDouble(127503);
    CHECK(state.TakeCombatStartLeftmostAttackDoubles() == 2);
    CHECK(state.TakeCombatStartLeftmostAttackDoubles() == 0);
    state.ArmCombatStartLeftmostAttackDouble(1);
    CHECK(state.TakeCombatStartLeftmostAttackDoubles() == 0);
}

TEST_CASE("[Season14] - nearest enemy stat copies stack and reset")
{
    Season14State state;
    state.ArmCombatStartNearestStats(119599);
    state.ArmCombatStartNearestStats(119599);
    CHECK(state.TakeCombatStartNearestStats() == 2);
    CHECK(state.TakeCombatStartNearestStats() == 0);
    state.ArmCombatStartNearestStats(1);
    CHECK(state.TakeCombatStartNearestStats() == 0);
}

TEST_CASE("[Season14] - random enemy health effect stacks and resets")
{
    Season14State state;
    state.ArmCombatStartRandomEnemySetHealth(104560);
    state.ArmCombatStartRandomEnemySetHealth(104560);
    CHECK(state.TakeCombatStartRandomEnemySetHealth() == 2);
    CHECK(state.TakeCombatStartRandomEnemySetHealth() == 0);
    state.ArmCombatStartRandomEnemySetHealth(1);
    CHECK(state.TakeCombatStartRandomEnemySetHealth() == 0);
}

TEST_CASE("[Season14] - refresh Blood Gem aura is player-owned")
{
    Season14State state;
    CHECK(!state.HasShopBloodGemsOnRefresh());
    state.ArmShopBloodGemsOnRefresh(126676);
    CHECK(state.HasShopBloodGemsOnRefresh());
    state.ArmShopBloodGemsOnRefresh(1);
    CHECK(state.HasShopBloodGemsOnRefresh());
}

TEST_CASE("[Season14] - mixed combat-start effects consume only their own entries")
{
    Season14State state;
    state.ArmCombatStartLeftmostAttackDouble(127503);
    state.ArmCombatStartNearestStats(119599);
    state.ArmCombatStartRandomEnemySetHealth(104560);
    state.ArmCombatStartLeftmostAttackDouble(127503);

    CHECK(state.TakeCombatStartLeftmostAttackDoubles() == 2);
    CHECK(state.TakeCombatStartNearestStats() == 1);
    CHECK(state.TakeCombatStartRandomEnemySetHealth() == 1);
    CHECK(state.TakeCombatStartLeftmostAttackDoubles() == 0);
    CHECK(state.TakeCombatStartNearestStats() == 0);
    CHECK(state.TakeCombatStartRandomEnemySetHealth() == 0);
}

TEST_CASE("[Season14] - combat-start Beetle casts stack and reset")
{
    Season14State state;
    state.ArmCombatStartBeetles(110401);
    state.ArmCombatStartBeetles(110401);
    CHECK(state.TakeCombatStartBeetles() == 2);
    CHECK(state.TakeCombatStartBeetles() == 0);
    state.ArmCombatStartBeetles(1);
    CHECK(state.TakeCombatStartBeetles() == 0);
}

TEST_CASE("[Season14] - next-combat reward stacks casts and maps both combat perspectives")
{
    Season14State winner;
    winner.ArmNextCombatReward(105267);
    winner.ArmNextCombatReward(105267);
    winner.ResolveNextCombatReward(BattleResult::PLAYER1_WIN, true);
    CHECK(winner.TakeNextTurnGold() == 6);

    Season14State loser;
    loser.ArmNextCombatReward(105267);
    loser.ResolveNextCombatReward(BattleResult::PLAYER1_WIN, false);
    CHECK(loser.TakeNextTurnGold() == 0);

    Season14State tied;
    tied.ArmNextCombatReward(105267);
    tied.ResolveNextCombatReward(BattleResult::DRAW, true);
    CHECK(tied.TakeNextTurnGold() == 1);
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
