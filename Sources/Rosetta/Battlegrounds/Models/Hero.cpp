// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Models/Hero.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

#include <algorithm>

namespace RosettaStone::Battlegrounds
{
void Hero::Initialize(const Card& heroCard)
{
    card = heroCard;
    health = heroCard.gameTags.at(GameTag::HEALTH);
}

HeroDamageEvent Hero::TakeDamage(Player& player, int amount,
                                 HeroDamageSource source)
{
    HeroDamageEvent event;
    event.requested = std::max(0, amount);
    event.source = source;
    // A resolved Ice Block prevents all subsequent damage in its active
    // damage window. Keep this before armor accounting: immunity does not
    // consume armor or dispatch hero-damage listeners.
    if (player.season14.iceBlockImmune)
        return event;
    const int absorbed = std::min(player.armor, event.requested);
    player.armor -= absorbed;
    event.absorbedByArmor = absorbed;
    event.healthLost = event.requested - absorbed;
    // Safety Patch casts the Battlegrounds Ice Block secret.  Secrets protect
    // against hostile/combat damage, not a player's own recruit-phase
    // payment.  Keep this source check beside the lethal boundary so a
    // lethal self-damage effect cannot consume the one-shot protection.
    if (source == HeroDamageSource::COMBAT_OPPONENT &&
        health - event.healthLost <= 0)
    {
        for (std::size_t i = 0; i < player.season14.trinkets.size(); ++i)
        {
            auto& effect = player.season14.trinkets[i];
            if (!effect.active || effect.remainingUses == 0) continue;
            const auto behavior = FindTrinketBehavior(
                Cards::FindCardByDbfID(effect.dbfID).id);
            if (behavior.effect != TrinketEffect::SAFETY_PATCH) continue;
            if (!player.season14.ConsumeEffect(player.season14.trinkets, i))
                continue;
            player.season14.iceBlockImmune = true;
            event.healthLost = 0;
            return event;
        }
    }
    health -= event.healthLost;
    // A hero-damage trigger means actual Health loss.  In particular, an
    // armor-only hit must not wake Tichondrius-like effects.
    if (event.healthLost > 0)
    {
        player.DispatchHeroDamage(event);
    }
    if (health <= 0)
    {
        player.ProcessDefeat();
    }
    return event;
}
}  // namespace RosettaStone::Battlegrounds
