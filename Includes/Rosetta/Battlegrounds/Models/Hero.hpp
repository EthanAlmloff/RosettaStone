// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_HERO_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HERO_HPP

#include <Rosetta/Battlegrounds/Cards/Card.hpp>

namespace RosettaStone::Battlegrounds
{
//! The gameplay origin of damage dealt to a Battlegrounds hero.
//! Keeping this on the event (rather than inferring it from the current phase)
//! prevents recruit self-damage from being confused with combat damage.
enum class HeroDamageSource
{
    RECRUIT_SELF,
    COMBAT_OPPONENT,
};

struct HeroDamageEvent
{
    int requested = 0;
    int absorbedByArmor = 0;
    int healthLost = 0;
    HeroDamageSource source = HeroDamageSource::RECRUIT_SELF;
};

//! 
//! \brief Hero class.
//!
//! A hero is a character in the Warcraft universe representing the player.
//!
class Hero
{
 public:
    //! Initializes a Hero instance and assigns the information.
    //! \param heroCard The card of a Hero instance.
    void Initialize(const Card& heroCard);

    //! Takes damage to the hero.
    //! \param player The owner of the hero.
    //! \param amount The amount of damage.
    HeroDamageEvent TakeDamage(
        Player& player, int amount,
        HeroDamageSource source = HeroDamageSource::RECRUIT_SELF);

    Card card;
    int health = 0;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_HERO_HPP
