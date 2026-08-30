// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>

#include <map>
#include <array>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - verified Patch 36.4 batch")
{
    const auto fortitude =
        FindDarkGiftBehavior("BG36_MidGameEffect_000t73");
    CHECK(fortitude.effect == DarkGiftEffect::TARGET_STATS);
    CHECK(fortitude.attack == 5);
    CHECK(fortitude.health == 5);
    CHECK(fortitude.uses == 1);

    const auto titan = FindDarkGiftBehavior("BG36_MidGameEffect_000t72");
    CHECK(titan.effect == DarkGiftEffect::TARGET_STATS);
    CHECK(titan.attack == 1000);
    CHECK(titan.health == 0);

    const auto talons = FindDarkGiftBehavior("BG36_MidGameEffect_000t13");
    CHECK(talons.effect == DarkGiftEffect::TARGET_KEYWORDS);
    CHECK(talons.divineShield);
    CHECK(talons.windfury);

    const auto toxicity = FindDarkGiftBehavior("BG36_MidGameEffect_000t69");
    CHECK(toxicity.effect == DarkGiftEffect::TARGET_KEYWORDS);
    CHECK(toxicity.venomous);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - direct target family")
{
    struct Expected
    {
        const char* id;
        DarkGiftEffect effect;
        int attackMultiplier;
        int healthMultiplier;
    };
    constexpr std::array expected{
        Expected{ "BG36_MidGameEffect_000t14", DarkGiftEffect::TARGET_GOLDEN,
                  1, 1 },
        Expected{ "BG36_MidGameEffect_000t12", DarkGiftEffect::TARGET_REBORN,
                  1, 1 },
        Expected{ "BG36_MidGameEffect_000t79", DarkGiftEffect::TARGET_STEALTH,
                  1, 1 },
        Expected{ "BG36_MidGameEffect_000t7",
                  DarkGiftEffect::START_COMBAT_STATS, 1, 2 },
        Expected{ "BG36_MidGameEffect_000t71",
                  DarkGiftEffect::START_COMBAT_STATS, 2, 1 },
        Expected{ "BG36_MidGameEffect_000t81",
                  DarkGiftEffect::START_COMBAT_STATS, 3, 3 },
    };

    for (const auto& item : expected)
    {
        const auto behavior = FindDarkGiftBehavior(item.id);
        CHECK(behavior.effect == item.effect);
        CHECK(behavior.uses == 1);
        CHECK(behavior.startCombatAttackMultiplier == item.attackMultiplier);
        CHECK(behavior.startCombatHealthMultiplier == item.healthMultiplier);
    }

    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());

    Minion golden(base);
    CHECK(DarkGiftTargetIsLegal(
        golden, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));
    REQUIRE(ApplyDarkGift(
        golden, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));
    CHECK(golden.IsGolden());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        golden, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));

    Minion reborn(base);
    CHECK(DarkGiftTargetIsLegal(
        reborn, FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));
    REQUIRE(ApplyDarkGift(
        reborn, FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));
    CHECK(reborn.HasReborn());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        reborn, FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));

    Minion stealth(base);
    CHECK(DarkGiftTargetIsLegal(
        stealth, FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));
    REQUIRE(ApplyDarkGift(
        stealth, FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));
    CHECK(stealth.HasStealth());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        stealth, FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));

    Minion combat(base);
    REQUIRE(ApplyDarkGift(
        combat, FindDarkGiftBehavior("BG36_MidGameEffect_000t81")));
    combat.ApplyStartCombatStatMultipliers();
    CHECK(combat.GetAttack() == base.GetAttack() * 3);
    CHECK(combat.GetHealth() == base.GetHealth() * 3);
    // The multiplier is consumed once even if a caller accidentally invokes
    // the start hook more than once for the same combat copy.
    combat.ApplyStartCombatStatMultipliers();
    CHECK(combat.GetAttack() == base.GetAttack() * 3);
    CHECK(combat.GetHealth() == base.GetHealth() * 3);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - unsupported is fail closed")
{
    CHECK(FindDarkGiftBehavior("BG36_MidGameEffect_000t4").effect ==
          DarkGiftEffect::NONE);

    std::map<std::string, CardDef> cards;
    DarkGiftBehaviors::AddAll(cards);
    CHECK(cards.size() == 10);
    CHECK(cards.contains("BG36_MidGameEffect_000t73"));
    CHECK(cards.contains("BG36_MidGameEffect_000t72"));
    CHECK(cards.contains("BG36_MidGameEffect_000t13"));
    CHECK(cards.contains("BG36_MidGameEffect_000t69"));
    CHECK(cards.contains("BG36_MidGameEffect_000t14"));
    CHECK(cards.contains("BG36_MidGameEffect_000t12"));
    CHECK(cards.contains("BG36_MidGameEffect_000t79"));
    CHECK(cards.contains("BG36_MidGameEffect_000t7"));
    CHECK(cards.contains("BG36_MidGameEffect_000t71"));
    CHECK(cards.contains("BG36_MidGameEffect_000t81"));
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - target application is reusable")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);

    CHECK(ApplyDarkGift(
        minion, FindDarkGiftBehavior("BG36_MidGameEffect_000t73")));
    CHECK(minion.GetAttack() == base.GetAttack() + 5);
    CHECK(minion.GetHealth() == base.GetHealth() + 5);

    CHECK(ApplyDarkGift(
        minion, FindDarkGiftBehavior("BG36_MidGameEffect_000t13")));
    CHECK(minion.HasDivineShield());
    CHECK(minion.HasWindfury());

    CHECK(ApplyDarkGift(
        minion, FindDarkGiftBehavior("BG36_MidGameEffect_000t69")));
    CHECK(minion.HasVenomous());

    Minion destroyed(base);
    destroyed.TakeDamage(1000);
    CHECK(destroyed.IsDestroyed());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        destroyed, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));
    CHECK_FALSE(ApplyDarkGift(
        destroyed, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));

    CHECK_FALSE(ApplyDarkGift(minion, {}));
    auto spent = FindDarkGiftBehavior("BG36_MidGameEffect_000t73");
    spent.uses = 0;
    CHECK_FALSE(ApplyDarkGift(minion, spent));
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - acquisition and one-use lifecycle")
{
    Season14State state;
    state.BeginDecision(
        Season14Decision::DARK_GIFT_SELECTION,
        { Season14Offering{ 133421, 1 } }); // BG36_MidGameEffect_000t73.

    REQUIRE(state.SelectDecision(0));
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t73");
    REQUIRE(behavior.effect != DarkGiftEffect::NONE);
    state.AddDarkGift({ 133421, behavior.uses, true });
    REQUIRE(state.darkGifts.size() == 1);

    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);
    REQUIRE(ApplyDarkGift(minion, behavior));
    CHECK(minion.GetAttack() == base.GetAttack() + 5);
    CHECK(minion.GetHealth() == base.GetHealth() + 5);
    REQUIRE(state.ConsumeEffect(state.darkGifts, 0));
    CHECK(state.darkGifts[0].remainingUses == 0);
    CHECK_FALSE(state.darkGifts[0].active);

    // A spent gift cannot be applied a second time through the lifecycle.
    CHECK_FALSE(state.ConsumeEffect(state.darkGifts, 0));
}
