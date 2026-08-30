// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>

namespace RosettaStone::Battlegrounds
{
DarkGiftBehavior FindDarkGiftBehavior(std::string_view id)
{
    if (id == "BG36_MidGameEffect_000t73") // Fortitude: +5/+5.
    {
        return { DarkGiftEffect::TARGET_STATS, 5, 5, false, false, false,
                 1 };
    }
    if (id == "BG36_MidGameEffect_000t72") // Titanic Strength: +1000 Attack.
    {
        return { DarkGiftEffect::TARGET_STATS, 1000, 0, false, false, false,
                 1 };
    }
    if (id == "BG36_MidGameEffect_000t13") // Harpy's Talons: DS/Windfury.
    {
        return { DarkGiftEffect::TARGET_KEYWORDS, 0, 0, true, true, false,
                 1 };
    }
    if (id == "BG36_MidGameEffect_000t69") // Toxicity: Venomous.
    {
        return { DarkGiftEffect::TARGET_KEYWORDS, 0, 0, false, false, true,
                 1 };
    }
    if (id == "BG36_MidGameEffect_000t14") // Gilding: golden, no triple.
    {
        return { DarkGiftEffect::TARGET_GOLDEN, 0, 0, false, false, false,
                 1, true, false, false, 1, 1 };
    }
    if (id == "BG36_MidGameEffect_000t12") // Persisting Horror: Reborn.
    {
        return { DarkGiftEffect::TARGET_REBORN, 0, 0, false, false, false,
                 1, false, true, false, 1, 1 };
    }
    if (id == "BG36_MidGameEffect_000t79") // Furtiveness: Stealth.
    {
        return { DarkGiftEffect::TARGET_STEALTH, 0, 0, false, false, false,
                 1, false, false, true, 1, 1 };
    }
    if (id == "BG36_MidGameEffect_000t7") // Resistance: double Health at combat start.
    {
        return { DarkGiftEffect::START_COMBAT_STATS, 0, 0, false, false,
                 false, 1, false, false, false, 1, 2 };
    }
    if (id == "BG36_MidGameEffect_000t71") // Hostility: double Attack at combat start.
    {
        return { DarkGiftEffect::START_COMBAT_STATS, 0, 0, false, false,
                 false, 1, false, false, false, 2, 1 };
    }
    if (id == "BG36_MidGameEffect_000t81") // Transcendence: triple stats at combat start.
    {
        return { DarkGiftEffect::START_COMBAT_STATS, 0, 0, false, false,
                 false, 1, false, false, false, 3, 3 };
    }
    return {};
}

bool DarkGiftTargetIsLegal(const Minion& target,
                           const DarkGiftBehavior& behavior)
{
    // A stale destroyed entry can remain in a zone until death processing
    // compacts it.  It is never a legal friendly target for a persistent
    // effect, and rejecting it here keeps masks and execution identical.
    if (behavior.effect == DarkGiftEffect::NONE || behavior.uses == 0 ||
        target.IsDestroyed())
    {
        return false;
    }
    switch (behavior.effect)
    {
        case DarkGiftEffect::TARGET_GOLDEN:
            return behavior.golden && target.CanMakeGolden();
        case DarkGiftEffect::TARGET_REBORN:
            return behavior.reborn && !target.HasReborn();
        case DarkGiftEffect::TARGET_STEALTH:
            return behavior.stealth && !target.HasStealth();
        case DarkGiftEffect::START_COMBAT_STATS:
            return behavior.startCombatAttackMultiplier >= 1 &&
                   behavior.startCombatHealthMultiplier >= 1;
        case DarkGiftEffect::TARGET_STATS:
            return behavior.attack != 0 || behavior.health != 0;
        case DarkGiftEffect::TARGET_KEYWORDS:
            return behavior.divineShield || behavior.windfury ||
                   behavior.venomous;
        case DarkGiftEffect::NONE:
            return false;
    }
    return false;
}

bool ApplyDarkGift(Minion& target, const DarkGiftBehavior& behavior)
{
    if (!DarkGiftTargetIsLegal(target, behavior))
    {
        return false;
    }

    if (behavior.effect == DarkGiftEffect::TARGET_STATS)
    {
        target.SetAttack(target.GetAttack() + behavior.attack);
        target.SetHealth(target.GetHealth() + behavior.health);
    }

    if (behavior.effect == DarkGiftEffect::TARGET_KEYWORDS)
    {
        if (behavior.divineShield)
        {
            target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
        }
        if (behavior.windfury)
        {
            target.SetGameTag(GameTag::WINDFURY, 1);
        }
        if (behavior.venomous)
        {
            target.SetGameTag(GameTag::VENOMOUS, 1);
        }
    }

    if (behavior.effect == DarkGiftEffect::TARGET_GOLDEN)
    {
        return behavior.golden && target.MakeGolden();
    }

    if (behavior.effect == DarkGiftEffect::TARGET_REBORN)
    {
        if (!behavior.reborn || target.HasReborn())
        {
            return false;
        }
        target.SetReborn(true);
        return true;
    }

    if (behavior.effect == DarkGiftEffect::TARGET_STEALTH)
    {
        if (!behavior.stealth || target.HasStealth())
        {
            return false;
        }
        target.SetGameTag(GameTag::STEALTH, 1);
        return true;
    }

    if (behavior.effect == DarkGiftEffect::START_COMBAT_STATS)
    {
        if (behavior.startCombatAttackMultiplier < 1 ||
            behavior.startCombatHealthMultiplier < 1)
        {
            return false;
        }
        target.SetStartCombatStatMultipliers(
            behavior.startCombatAttackMultiplier,
            behavior.startCombatHealthMultiplier);
        return true;
    }

    return true;
}

void DarkGiftBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // These entities are persistent effects, not ordinary playable cards.
    // An empty CardDef marks their behavior registration; ApplyDarkGift is
    // the only executor and is reached through the semantic bridge action.
    for (const auto* id : { "BG36_MidGameEffect_000t73",
                            "BG36_MidGameEffect_000t72",
                            "BG36_MidGameEffect_000t13",
                            "BG36_MidGameEffect_000t69",
                            "BG36_MidGameEffect_000t14",
                            "BG36_MidGameEffect_000t12",
                            "BG36_MidGameEffect_000t79",
                            "BG36_MidGameEffect_000t7",
                            "BG36_MidGameEffect_000t71",
                            "BG36_MidGameEffect_000t81" })
    {
        cards.emplace(id, CardDef{});
    }
}
}  // namespace RosettaStone::Battlegrounds
