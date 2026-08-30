// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Models/Hero.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>

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
    const int absorbed = std::min(player.armor, event.requested);
    player.armor -= absorbed;
    event.absorbedByArmor = absorbed;
    event.healthLost = event.requested - absorbed;
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
