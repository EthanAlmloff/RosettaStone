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

void Hero::TakeDamage(Player& player, int amount)
{
    const int absorbed = std::min(player.armor, std::max(0, amount));
    player.armor -= absorbed;
    amount -= absorbed;
    health -= amount;
    if (health <= 0)
    {
        player.ProcessDefeat();
    }
}
}  // namespace RosettaStone::Battlegrounds
