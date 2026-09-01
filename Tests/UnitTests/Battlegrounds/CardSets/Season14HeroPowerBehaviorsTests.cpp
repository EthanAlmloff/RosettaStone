// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/Season14HeroPowerBehaviorsBatch6.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Season14HeroPowerBehaviors] - Nine Frogs is registered")
{
    const auto* entry = FindSeason14HeroPowerBehavior(110472);
    REQUIRE(entry != nullptr);
    REQUIRE(FindSeason14HeroPowerBehavior("BG28_HERO_801p") == entry);
    CHECK(entry->kind == Season14HeroPowerKind::RANDOM_TAVERN_SPELL);
    CHECK(entry->cost == 1);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Void Power has turn-seven Discover payload")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS_BATCH6.size() == 1);
    const auto* entry = FindSeason14HeroPowerBehaviorBatch6("BG36_HERO_101p");
    REQUIRE(entry != nullptr);
    CHECK(FindSeason14HeroPowerBehaviorBatch6(132581) == entry);
    CHECK(entry->kind == Season14HeroPowerBatch6Kind::VOID_POWER);
    CHECK(entry->cost == 0);
    CHECK(!entry->passive);

    Season14HeroPowerBatch6State state{};
    for (int turn = 1; turn < 7; ++turn)
        CHECK(!ResolveVoidPowerBeginTurn(132581, state));
    CHECK(ResolveVoidPowerBeginTurn(132581, state));
    CHECK(state.turnNumber == 7);
    CHECK(state.discoverReady);
    CHECK(ConsumeVoidPowerDiscover(state));
    CHECK(!state.discoverReady);
    CHECK(state.discoverOffered);
    CHECK(!ResolveVoidPowerBeginTurn(132581, state));
    CHECK(!ConsumeVoidPowerDiscover(state));

    state = {};
    state.discoverReady = true;
    CHECK(!ConsumeVoidPowerDiscover(state));
    RestoreVoidPowerDiscoverReady(state);
    CHECK(state.discoverReady);

    Season14State modal{};
    modal.BeginOfferingDecision(
        Season14Decision::DISCOVER, 0, 132581,
        {{615, 0, 138879}, {616, 0, 138880}, {617, 0, 138881}});
    CHECK(modal.pendingDecision == Season14Decision::DISCOVER);
    CHECK(modal.pendingSourceCardDbfID == 132581);
    REQUIRE(modal.pendingOfferings.size() == 3);
    CHECK(modal.pendingOfferings[0].dbfID == 615);
    CHECK(modal.pendingOfferings[0].darkGiftDbfID == 138879);
    CHECK(modal.SelectDecision(1));
    CHECK(modal.pendingDecision == Season14Decision::NONE);
    CHECK(modal.pendingOfferings.empty());
}

TEST_CASE("[Season14HeroPowerBehaviors] - batch has exact unique IDs")
{
    CHECK(SEASON14_HERO_POWER_BEHAVIORS.size() == 24);

    const auto* boon = FindSeason14HeroPowerBehavior(57562);
    REQUIRE(boon != nullptr);
    CHECK(FindSeason14HeroPowerBehavior("TB_BaconShop_HP_010") == boon);
    CHECK(boon->kind == Season14HeroPowerKind::BOON_OF_LIGHT);
    CHECK(boon->cost == 2);
    CHECK(!boon->passive);
    const auto* sharpen = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_001");
    REQUIRE(sharpen != nullptr);
    CHECK(sharpen->dbfID == 57567);
    CHECK(sharpen->kind == Season14HeroPowerKind::SHARPEN_BLADES);
    CHECK(sharpen->cost == 1);
    const auto* buried = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_074");
    REQUIRE(buried != nullptr);
    CHECK(buried->dbfID == 62250);
    CHECK(buried->kind == Season14HeroPowerKind::BURIED_TREASURE);
    const auto* firstKill = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_053");
    REQUIRE(firstKill != nullptr);
    CHECK(firstKill->dbfID == 60381);
    CHECK(firstKill->kind == Season14HeroPowerKind::FIRST_KILL_COPY);
    const auto* seeLight = FindSeason14HeroPowerBehavior("BG20_HERO_101p");
    REQUIRE(seeLight != nullptr);
    CHECK(seeLight->dbfID == 70957);
    CHECK(seeLight->kind == Season14HeroPowerKind::SEE_THE_LIGHT);
    CHECK(seeLight->cost == 2);
    CHECK(!seeLight->passive);
    const auto* luckyRoll = FindSeason14HeroPowerBehavior("BG28_HERO_400p");
    REQUIRE(luckyRoll != nullptr);
    CHECK(luckyRoll->dbfID == 105315);
    CHECK(luckyRoll->kind == Season14HeroPowerKind::LUCKY_ROLL);
    CHECK(luckyRoll->cost == 1);
    CHECK(!luckyRoll->passive);
    const auto* brick = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_040");
    REQUIRE(brick != nullptr);
    CHECK(brick->dbfID == 59832);
    CHECK(brick->kind == Season14HeroPowerKind::BRICK_BY_BRICK);
    CHECK(brick->cost == 0);
    CHECK(!brick->passive);
    const auto* rich = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_046");
    REQUIRE(rich != nullptr);
    CHECK(rich->dbfID == 60216);
    CHECK(rich->kind == Season14HeroPowerKind::GONNA_BE_RICH);
    CHECK(rich->cost == 0);
    CHECK(!rich->passive);
    const auto* explorer = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_047");
    REQUIRE(explorer != nullptr);
    CHECK(explorer->dbfID == 60217);
    CHECK(explorer->kind == Season14HeroPowerKind::LEAD_EXPLORER);
    CHECK(explorer->cost == 1);
    CHECK(!explorer->passive);
    const auto* cloning = FindSeason14HeroPowerBehavior("BG31_HERO_005p");
    REQUIRE(cloning != nullptr);
    CHECK(cloning->dbfID == 117410);
    CHECK(cloning->kind == Season14HeroPowerKind::CLONING_GALLERY);
    CHECK(cloning->cost == 0);
    CHECK(!cloning->passive);

    for (std::size_t i = 0; i < SEASON14_HERO_POWER_BEHAVIORS.size(); ++i)
    {
        const auto& entry = SEASON14_HERO_POWER_BEHAVIORS[i];
        REQUIRE(FindSeason14HeroPowerBehavior(entry.id) != nullptr);
        REQUIRE(FindSeason14HeroPowerBehavior(entry.dbfID) != nullptr);
        CHECK(FindSeason14HeroPowerBehavior(entry.id)->dbfID == entry.dbfID);
        for (std::size_t j = i + 1; j < SEASON14_HERO_POWER_BEHAVIORS.size();
             ++j)
        {
            CHECK(entry.id != SEASON14_HERO_POWER_BEHAVIORS[j].id);
            CHECK(entry.dbfID != SEASON14_HERO_POWER_BEHAVIORS[j].dbfID);
        }
    }
}

TEST_CASE("[Season14HeroPowerBehaviors] - See the Light tavern target contract")
{
    CHECK(Season14HeroPowerUsesTavernTarget(70957));
    CHECK(!Season14HeroPowerUsesTavernTarget(57562));
    CHECK(Season14HeroPowerTavernTargetIsLegal(70957, true, 1));
    CHECK(!Season14HeroPowerTavernTargetIsLegal(70957, false, 1));
    CHECK(!Season14HeroPowerTavernTargetIsLegal(70957, true, 0));
    CHECK(!Season14HeroPowerTavernTargetIsLegal(57562, true, 3));
}

TEST_CASE("[Season14HeroPowerBehaviors] - See the Light registry metadata is exact")
{
    const auto* entry = FindSeason14HeroPowerBehavior(70957);
    REQUIRE(entry != nullptr);
    CHECK(entry->id == "BG20_HERO_101p");
    CHECK(entry->kind == Season14HeroPowerKind::SEE_THE_LIGHT);
    CHECK(entry->cost == 2);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Natural Balance is a typed active family")
{
    const auto* entry = FindSeason14HeroPowerBehavior(68130);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerKind::NATURAL_BALANCE);
    CHECK(entry->cost == 2);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Spirit Swap is a two-target family")
{
    const auto* entry = FindSeason14HeroPowerBehavior(71464);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerKind::SPIRIT_SWAP);
    CHECK(entry->cost == 0);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Galakrond Greed is a Tavern discover")
{
    const auto* entry = FindSeason14HeroPowerBehavior(57555);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerKind::GALAKROND_GREED);
    CHECK(entry->cost == 1);
    CHECK(!entry->passive);
    CHECK(Season14HeroPowerUsesGalakrondGreed(57555));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Naga Conquest is a three-card Discover")
{
    const auto* entry = FindSeason14HeroPowerBehavior(80007);
    REQUIRE(entry != nullptr);
    CHECK(FindSeason14HeroPowerBehavior("BG22_HERO_007p2") == entry);
    CHECK(entry->kind == Season14HeroPowerKind::NAGA_CONQUEST);
    CHECK(entry->cost == 1);
    CHECK(!entry->passive);
    CHECK(Season14HeroPowerUsesNagaConquest(80007));
    CHECK(!Season14HeroPowerUsesNagaConquest(57555));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Reclaimed Souls registry")
{
    const auto* entry = FindSeason14HeroPowerBehavior(89294);
    REQUIRE(entry != nullptr);
    CHECK(entry->id == "BG23_HERO_306p");
    CHECK(entry->kind == Season14HeroPowerKind::RECLAIMED_SOULS);
    CHECK(entry->cost == 2);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Reborn Rites registry")
{
    const auto* entry = FindSeason14HeroPowerBehavior(58040);
    REQUIRE(entry != nullptr);
    CHECK(entry->id == "TB_BaconShop_HP_024");
    CHECK(entry->kind == Season14HeroPowerKind::REBORN_RITES);
    CHECK(entry->cost == 0);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Imprison registry")
{
    const auto* entry = FindSeason14HeroPowerBehavior(61919);
    REQUIRE(entry != nullptr);
    CHECK(entry->id == "TB_BaconShop_HP_068");
    CHECK(entry->kind == Season14HeroPowerKind::IMPRISON);
    CHECK(entry->cost == 1);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Sign a New Artist registry")
{
    const auto* entry = FindSeason14HeroPowerBehavior(101346);
    REQUIRE(entry != nullptr);
    CHECK(entry->id == "BG25_HERO_105p");
    CHECK(entry->kind == Season14HeroPowerKind::SIGN_NEW_ARTIST);
    CHECK(entry->cost == 3);
    CHECK(entry->buddyDbfID == 101349);
    CHECK(!entry->passive);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Devour uses two distinct friendly targets")
{
    const auto* entry = FindSeason14HeroPowerBehavior(71914);
    REQUIRE(entry != nullptr);
    CHECK(entry->kind == Season14HeroPowerKind::DEVOUR);
    CHECK(entry->cost == 0);
    CHECK(!entry->passive);
    CHECK(Season14HeroPowerUsesDevour(71914));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Dungar flightpath countdowns")
{
    Season14State state{};
    CHECK(state.SelectFlightpath(75705));
    CHECK(state.flightpath.turnsRemaining == 2);
    CHECK(!state.AdvanceFlightpath());
    CHECK(state.AdvanceFlightpath());
    CHECK(state.flightpath.pathDbfID == 0);
    CHECK(state.flightpath.completedDbfID == 75705);
    CHECK(state.TakeCompletedFlightpath() == 75705);
    CHECK(state.TakeCompletedFlightpath() == 0);
    CHECK(state.SelectFlightpath(75706));
    for (int i = 0; i < 3; ++i) state.AdvanceFlightpath();
    CHECK(state.TakeCompletedFlightpath() == 75706);
    CHECK(!state.SelectFlightpath(999999));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Buried Treasure has four digs")
{
    Season14State state;
    state.SetHeroPower(62250, 1, true);
    for (int i = 0; i < 4; ++i) {
        CHECK(state.CanBuriedTreasureDig());
        state.RecordBuriedTreasureDig();
    }
    CHECK(!state.CanBuriedTreasureDig());
    state.SetHeroPower(62250, 1, true);
    CHECK(state.CanBuriedTreasureDig());
}

TEST_CASE("[Season14HeroPowerBehaviors] - first kill copy expires after combat")
{
    Season14State state;
    state.SetHeroPower(60381, 1, true);
    state.ArmFirstKillCopy();
    CHECK(state.firstKillCopyArmed);
    state.ExpireFirstKillCopy();
    CHECK(!state.firstKillCopyArmed);
    Minion copy;
    CHECK(!state.TakeFirstKillCopy(copy));
}

TEST_CASE("[Season14HeroPowerBehaviors] - passive families are explicit")
{
    const auto* patched = FindSeason14HeroPowerBehavior(59399);
    REQUIRE(patched != nullptr);
    CHECK(patched->kind == Season14HeroPowerKind::STARTING_HEALTH);
    CHECK(patched->passive);

    const auto* manastorm = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_054");
    REQUIRE(manastorm != nullptr);
    CHECK(manastorm->kind == Season14HeroPowerKind::ECONOMY_COST_AURA);
    CHECK(manastorm->passive);

    const auto* lighting = FindSeason14HeroPowerBehavior("TB_BaconShop_HP_085t");
    REQUIRE(lighting != nullptr);
    CHECK(lighting->kind == Season14HeroPowerKind::TAVERN_SPELL_AURA);
    CHECK(lighting->passive);

    const auto patchedModifiers = Season14HeroPowerBatch1Modifiers(59399);
    CHECK(patchedModifiers.StartingHealth(40) == 70);
    CHECK(patchedModifiers.StartingHealth(60) == 90);

    const auto manastormModifiers = Season14HeroPowerBatch1Modifiers(60405);
    CHECK(manastormModifiers.UpgradeCost(5) == 6);

    // Tavern Lighting buffs spell results; it is not a payment discount.
    CHECK(Season14HeroPowerBatch1Modifiers(122960).TavernSpellCost(3) == 3);
}

TEST_CASE("[Season14HeroPowerBehaviors] - no-target active effects are deterministic")
{
    Season14HeroPowerActivation activation{};

    CHECK(ResolveSeason14HeroPowerActivation(62269, 1, activation));
    CHECK(activation.goldDelta == 1);
    CHECK(activation.maxGoldDelta == 0);

    CHECK(ResolveSeason14HeroPowerActivation(62269, 4, activation));
    CHECK(activation.goldDelta == 4);

    Season14HeroPowerBatch1State piggyState{};
    CHECK(ResolveSeason14HeroPowerActivation(62269, 1, piggyState,
                                             activation));
    CHECK(activation.goldDelta == 1);
    CHECK(piggyState.piggyBankUsed);
    CHECK(!ResolveSeason14HeroPowerActivation(62269, 2, piggyState,
                                              activation));

    CHECK(ResolveSeason14HeroPowerActivation(116921, 1, activation));
    CHECK(activation.goldDelta == 0);
    CHECK(activation.maxGoldDelta == 1);

    CHECK(!ResolveSeason14HeroPowerActivation(59399, 1, activation));
    CHECK(!ResolveSeason14HeroPowerActivation(0, 1, activation));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Sharpen Blades scales with current-turn buys")
{
    Season14State state;
    state.SetHeroPower(57567, 1, true);
    CHECK(state.SharpenBladesStats() == std::pair{0, 0});
    state.OnBuyMinionSharpenBlades();
    CHECK(state.SharpenBladesStats() == std::pair{2, 1});
    state.OnBuyMinionSharpenBlades();
    state.OnBuyMinionSharpenBlades();
    CHECK(state.SharpenBladesStats() == std::pair{6, 3});
    state.BeginRecruitTurn();
    CHECK(state.SharpenBladesStats() == std::pair{0, 0});
    state.SetHeroPower(57567, 1, true);
    CHECK(state.SharpenBladesStats() == std::pair{0, 0});
}

TEST_CASE("[Season14HeroPowerBehaviors] - Brick by Brick grows only when unused")
{
    Season14HeroPowerBatch1State state{};
    Season14HeroPowerActivation activation{};
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    CHECK(ResolveSeason14HeroPowerActivation(59832, 2, state, activation));
    CHECK(activation.healthDelta == 3);
    CHECK(!ResolveSeason14HeroPowerActivation(59832, 2, state, activation));
    state = {};
    CHECK(ResolveSeason14HeroPowerActivation(59832, 1, state, activation));
    CHECK(activation.healthDelta == 2);
    CHECK(!ResolveSeason14HeroPowerActivation(59832, 1, state, activation));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Gonna Be Rich is once per game")
{
    Season14HeroPowerBatch1State state{};
    Season14HeroPowerActivation activation{};
    CHECK(ResolveSeason14HeroPowerActivation(60216, 1, state, activation));
    CHECK(activation.makeGolden);
    CHECK(state.gonnaBeRichUsed);
    CHECK(!ResolveSeason14HeroPowerActivation(60216, 2, state, activation));
    state = {};
    CHECK(ResolveSeason14HeroPowerActivation(60216, 2, state, activation));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Lead Explorer raises cost after use")
{
    Season14HeroPowerBatch1State state{};
    Season14HeroPowerActivation activation{};
    CHECK(ResolveSeason14HeroPowerActivation(60217, 1, state, activation));
    CHECK(activation.beginDiscover);
    CHECK(state.leadExplorerCostDelta == 1);
    CHECK(ResolveSeason14HeroPowerActivation(60217, 2, state, activation));
    CHECK(state.leadExplorerCostDelta == 2);
}

TEST_CASE("[Season14HeroPowerBehaviors] - Cloning Gallery is once per game")
{
    Season14State state;
    state.SetHeroPower(117410, 0, true);
    CHECK(!state.cloningGalleryUsed);
    state.cloningGalleryUsed = true;
    CHECK(state.cloningGalleryUsed);
    state.SetHeroPower(117410, 0, true);
    CHECK(!state.cloningGalleryUsed);
}

TEST_CASE("[Season14HeroPowerBehaviors] - King of Duality unlocks once on turn four")
{
    Season14HeroPowerBatch1State state{};
    Season14HeroPowerActivation activation{};
    CHECK(!ResolveSeason14HeroPowerActivation(129685, 3, state, activation));
    CHECK(ResolveSeason14HeroPowerActivation(129685, 4, state, activation));
    CHECK(activation.beginDiscover);
    CHECK(state.kingOfDualityOffered);
    CHECK(!ResolveSeason14HeroPowerActivation(129685, 4, state, activation));
}

TEST_CASE("[Season14HeroPowerBehaviors] - Brick by Brick preserves missed-turn growth")
{
    Season14HeroPowerBatch1State state{};
    Season14HeroPowerActivation activation{};
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    CHECK(ResolveSeason14HeroPowerActivation(59832, 1, state, activation));
    CHECK(activation.healthDelta == 2);

    // Using the power prevents growth on the immediately following turn.
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    CHECK(ResolveSeason14HeroPowerActivation(59832, 2, state, activation));
    CHECK(activation.healthDelta == 2);

    // Two consecutive unused turns each add one to the next activation.
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    ResolveSeason14HeroPowerBatch1Event(
        59832, Season14HeroPowerBatch1Event::BEGIN_TURN, state);
    CHECK(ResolveSeason14HeroPowerActivation(59832, 4, state, activation));
    CHECK(activation.healthDelta == 4);
}
