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
    return {};
}

bool ApplyDarkGift(Minion& target, const DarkGiftBehavior& behavior)
{
    if (behavior.effect == DarkGiftEffect::NONE || behavior.uses == 0)
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
                            "BG36_MidGameEffect_000t69" })
    {
        cards.emplace(id, CardDef{});
    }
}
}  // namespace RosettaStone::Battlegrounds
