// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>

#include <array>
#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - verified Patch 36.4 batch")
{
    const auto fortitude = FindDarkGiftBehavior("BG36_MidGameEffect_000t73");
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

    for (const auto* id : {"BG36_MidGameEffect_000t64",
                           "BG36_MidGameEffect_000t74",
                           "BG36_MidGameEffect_000t75"})
    {
        const auto behavior = FindDarkGiftBehavior(id);
        CHECK(behavior.effect == DarkGiftEffect::PLAY_CARD_STATS);
        CHECK(DarkGiftTargetIsLegal(Cards::FindCardByID("BGS_039"), behavior));
    }
    const auto endTurn = FindDarkGiftBehavior("BG36_MidGameEffect_000t10");
    CHECK(endTurn.effect == DarkGiftEffect::END_TURN_BATTLECRY);
    const auto jaws = FindDarkGiftBehavior("BG36_MidGameEffect_000t16");
    CHECK(jaws.effect == DarkGiftEffect::START_COMBAT_DEATHRATTLE);
    const auto admiration = FindDarkGiftBehavior("BG36_MidGameEffect_000t9");
    CHECK(admiration.effect == DarkGiftEffect::START_COMBAT_LEFT_ATTACK);
    const auto invulnerability = FindDarkGiftBehavior("BG36_MidGameEffect_000t60");
    CHECK(invulnerability.effect == DarkGiftEffect::IMMUNE_WHILE_ATTACKING);
    CHECK(FindDarkGiftBehavior("BG36_MidGameEffect_000t").effect ==
          DarkGiftEffect::DEATHRATTLE_STATS);
    const auto offensive = FindDarkGiftBehavior("BG36_MidGameEffect_000t");
    CHECK(offensive.attack == 10);
    const auto defensive = FindDarkGiftBehavior("BG36_MidGameEffect_000t2");
    CHECK(defensive.effect == DarkGiftEffect::DEATHRATTLE_STATS);
    CHECK(defensive.attack == 0);
    CHECK(defensive.health == 10);
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
        Expected{ "BG36_MidGameEffect_000t14", DarkGiftEffect::TARGET_GOLDEN, 1,
                  1 },
        Expected{ "BG36_MidGameEffect_000t12", DarkGiftEffect::TARGET_REBORN, 1,
                  1 },
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
    REQUIRE(ApplyDarkGift(golden,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));
    CHECK(golden.IsGolden());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        golden, FindDarkGiftBehavior("BG36_MidGameEffect_000t14")));

    Minion reborn(base);
    CHECK(DarkGiftTargetIsLegal(
        reborn, FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));
    REQUIRE(ApplyDarkGift(reborn,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));
    CHECK(reborn.HasReborn());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        reborn, FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));

    Minion stealth(base);
    CHECK(DarkGiftTargetIsLegal(
        stealth, FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));
    REQUIRE(ApplyDarkGift(stealth,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));
    CHECK(stealth.HasStealth());
    CHECK_FALSE(DarkGiftTargetIsLegal(
        stealth, FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));

    Minion combat(base);
    REQUIRE(ApplyDarkGift(combat,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t81")));
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
    CHECK(FindDarkGiftBehavior("BG36_MidGameEffect_000t11").effect ==
          DarkGiftEffect::NONE);

    std::map<std::string, CardDef> cards;
    DarkGiftBehaviors::AddAll(cards);
    CHECK(cards.size() == 29);
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
    CHECK(cards.contains("BG36_MidGameEffect_000t16"));
    CHECK(cards.contains("BG36_MidGameEffect_000t9"));
    CHECK(cards.contains("BG36_MidGameEffect_000t60"));
    CHECK(cards.contains("BG36_MidGameEffect_000t64"));
    CHECK(cards.contains("BG36_MidGameEffect_000t74"));
    CHECK(cards.contains("BG36_MidGameEffect_000t75"));
    CHECK(cards.contains("BG36_MidGameEffect_000t10"));
    CHECK(cards.contains("BG36_MidGameEffect_000t"));
    CHECK(cards.contains("BG36_MidGameEffect_000t2"));
    CHECK(cards.contains("BG36_MidGameEffect_000t4"));
    CHECK(cards.contains("BG36_MidGameEffect_000t52"));
    CHECK(cards.contains("BG36_MidGameEffect_000t15"));
    CHECK(cards.contains("BG36_MidGameEffect_000t15e"));
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Jaws arms only minions with deathrattles")
{
    const Card plain = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(plain.id.empty());
    Minion minion(plain);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t16");
    CHECK_FALSE(DarkGiftTargetIsLegal(minion, behavior));

    minion.SetDeathrattleStatTransfer(1, 0);
    // The transfer setter is state-only and does not make a deathrattle; use
    // a real generated deathrattle definition for the positive target path.
    const Card deathrattle = Cards::FindCardByID("BGS_039");
    Minion armed(deathrattle);
    armed.AddDarkGiftDeathrattleTask(SimpleTasks::FreeRefreshTask{1});
    CHECK(armed.HasDeathrattle());
    CHECK(DarkGiftTargetIsLegal(armed, behavior));
    REQUIRE(ApplyDarkGift(armed, behavior));
    CHECK(armed.HasStartCombatDeathrattleTrigger());
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Admiration copies immediate left attack once")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion left(base);
    Minion target(base);
    left.SetAttack(17);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t9");
    REQUIRE(ApplyDarkGift(target, behavior));
    CHECK(target.HasStartCombatLeftAttack());
    const int before = target.GetAttack();
    target.ApplyStartCombatLeftAttack(left);
    CHECK(target.GetAttack() == before + 17);
    CHECK_FALSE(target.HasStartCombatLeftAttack());
    target.ApplyStartCombatLeftAttack(left);
    CHECK(target.GetAttack() == before + 17);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Invulnerability blocks damage while attacking")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion target(base);
    Minion source(base);
    target.SetHealth(20);
    source.SetAttack(7);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t60");
    REQUIRE(ApplyDarkGift(target, behavior));
    target.SetAttacking(true);
    target.TakeDamage(source);
    CHECK(target.GetHealth() == 20);
    target.SetAttacking(false);
    target.TakeDamage(source);
    CHECK(target.GetHealth() == 13);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Incubation doubles after two recruit turns")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion target(base);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t4");
    CHECK(behavior.effect == DarkGiftEffect::INCUBATION);
    CHECK(behavior.incubationTurns == 2);
    REQUIRE(ApplyDarkGift(target, behavior));
    CHECK(target.IncubationTurnsRemaining() == 2);
    const auto attack = target.GetAttack();
    const auto health = target.GetHealth();
    target.AdvanceIncubation();
    CHECK(target.IncubationTurnsRemaining() == 1);
    CHECK(target.GetAttack() == attack);
    CHECK(target.GetHealth() == health);
    target.AdvanceIncubation();
    CHECK(target.IncubationTurnsRemaining() == 0);
    CHECK(target.GetAttack() == attack * 2);
    CHECK(target.GetHealth() == health * 2);
    target.AdvanceIncubation();
    CHECK(target.GetAttack() == attack * 2);
    CHECK(target.GetHealth() == health * 2);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Toreth shield absorbs three hits")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion target(base);
    Minion attacker(base);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t15");
    CHECK(behavior.effect == DarkGiftEffect::TARGET_MULTI_HIT_DIVINE_SHIELD);
    CHECK(DarkGiftTargetIsLegal(target, behavior));
    REQUIRE(ApplyDarkGift(target, behavior));
    CHECK(target.DivineShieldHitsRemaining() == 3);
    const int health = target.GetHealth();
    target.TakeDamage(attacker);
    CHECK(target.DivineShieldHitsRemaining() == 2);
    target.TakeDamage(attacker);
    CHECK(target.DivineShieldHitsRemaining() == 1);
    target.TakeDamage(attacker);
    CHECK(target.DivineShieldHitsRemaining() == 0);
    CHECK(target.GetHealth() == health);
    CHECK_FALSE(target.IsDestroyed());

    // Removing the shield must also discard the multi-hit counter; a later
    // ordinary Divine Shield is only one hit.
    target.SetGameTag(GameTag::DIVINE_SHIELD, 0);
    target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
    CHECK(target.DivineShieldHitsRemaining() == 1);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - Fresh Perspective arms free refreshes")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);
    Minion second(base);
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t52");
    CHECK(behavior.effect == DarkGiftEffect::DEATHRATTLE_FREE_REFRESH);
    CHECK(DarkGiftTargetIsLegal(minion, behavior));
    REQUIRE(ApplyDarkGift(minion, behavior));
    REQUIRE(ApplyDarkGift(second, behavior));

    Player owner;
    minion.ActivateTask(PowerType::DEATHRATTLE, owner);
    second.ActivateTask(PowerType::DEATHRATTLE, owner);
    CHECK(owner.season14.HasFreeRefresh());
    CHECK(owner.season14.ConsumeFreeRefresh());
    CHECK(owner.season14.ConsumeFreeRefresh());
    CHECK(owner.season14.ConsumeFreeRefresh());
    CHECK(owner.season14.ConsumeFreeRefresh());
    CHECK_FALSE(owner.season14.HasFreeRefresh());
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - play card stat gifts persist")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t64")));
    minion.ApplyPlayCardStatBonus();
    CHECK(minion.GetAttack() == base.GetAttack() + 2);
    CHECK(minion.GetHealth() == base.GetHealth() + 2);
    minion.ApplyPlayCardStatBonus();
    CHECK(minion.GetAttack() == base.GetAttack() + 4);
    CHECK(minion.GetHealth() == base.GetHealth() + 4);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - end turn trigger requires battlecry")
{
    const auto behavior = FindDarkGiftBehavior("BG36_MidGameEffect_000t10");
    const Card plain = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(plain.id.empty());
    Minion minion(plain);
    CHECK_FALSE(DarkGiftTargetIsLegal(minion, behavior));
}

TEST_CASE(
    "[Battlegrounds : DarkGiftBehaviors] - target application is reusable")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);

    CHECK(ApplyDarkGift(minion,
                        FindDarkGiftBehavior("BG36_MidGameEffect_000t73")));
    CHECK(minion.GetAttack() == base.GetAttack() + 5);
    CHECK(minion.GetHealth() == base.GetHealth() + 5);

    CHECK(ApplyDarkGift(minion,
                        FindDarkGiftBehavior("BG36_MidGameEffect_000t13")));
    CHECK(minion.HasDivineShield());
    CHECK(minion.HasWindfury());

    CHECK(ApplyDarkGift(minion,
                        FindDarkGiftBehavior("BG36_MidGameEffect_000t69")));
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

TEST_CASE(
    "[Battlegrounds : DarkGiftBehaviors] - Gilding preserves instance state")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);

    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t73")));
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t13")));
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t69")));
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t12")));
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t79")));

    const int attack = minion.GetAttack();
    const int health = minion.GetHealth();
    REQUIRE(minion.MakeGolden());
    CHECK(minion.IsGolden());
    CHECK(minion.GetAttack() == attack);
    CHECK(minion.GetHealth() == health);
    CHECK(minion.HasDivineShield());
    CHECK(minion.HasWindfury());
    CHECK(minion.HasVenomous());
    CHECK(minion.HasReborn());
    CHECK(minion.HasStealth());
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - start combat gifts compose")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion minion(base);

    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t7")));
    REQUIRE(ApplyDarkGift(minion,
                          FindDarkGiftBehavior("BG36_MidGameEffect_000t71")));
    minion.ApplyStartCombatStatMultipliers();

    CHECK(minion.GetAttack() == base.GetAttack() * 2);
    CHECK(minion.GetHealth() == base.GetHealth() * 2);
}

TEST_CASE("[Battlegrounds : DarkGiftBehaviors] - sacrifice gifts arm stat transfer")
{
    const Card base = Cards::FindCardByID("BGS_039");
    REQUIRE_FALSE(base.id.empty());
    Minion offensive(base);
    Minion defensive(base);

    REQUIRE(ApplyDarkGift(
        offensive, FindDarkGiftBehavior("BG36_MidGameEffect_000t")));
    REQUIRE(ApplyDarkGift(
        defensive, FindDarkGiftBehavior("BG36_MidGameEffect_000t2")));
    CHECK(offensive.DeathrattleAttackTransfer() == 10);
    CHECK(offensive.DeathrattleHealthTransfer() == 0);
    CHECK(defensive.DeathrattleAttackTransfer() == 0);
    CHECK(defensive.DeathrattleHealthTransfer() == 10);
}

TEST_CASE(
    "[Battlegrounds : DarkGiftBehaviors] - acquisition and one-use lifecycle")
{
    Season14State state;
    state.BeginDecision(
        Season14Decision::DARK_GIFT_SELECTION,
        { Season14Offering{ 133421, 1 } });  // BG36_MidGameEffect_000t73.

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
