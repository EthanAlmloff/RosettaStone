// This code is based on Sabberstone project.
// Copyright (c) 2017-2019 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

#ifndef ROSETTASTONE_BATTLEGROUNDS_CARD_DEF_HPP
#define ROSETTASTONE_BATTLEGROUNDS_CARD_DEF_HPP

#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Common/Enums/CardEnums.hpp>

#include <map>

namespace RosettaStone::Battlegrounds
{
//! Behavior owned by an authoritative phase/lifecycle boundary rather than
//! by a fixed Power task graph. Explicit tagging prevents an empty CardDef
//! from being mistaken for an unimplemented entity.
enum class CardLifecycle : unsigned char
{
    NONE,
    BUDDY_APOSTLE,
    BUDDY_ECLIPSION,
    BUDDY_LUCIFRON,
    BUDDY_PIGEON_LORD,
    BUDDY_ELEMENTIUM_SQUIRREL_BOMB,
    // Solemn Serenader observes a successful targeted Hero Power in
    // Player::ApplyHeroPowerTarget.  Its effect is phase-owned rather than a
    // fixed Power task graph, but the CardDef must still identify that owner.
    BUDDY_SOLEMN_SERENADER,
    // Choose-One minions and their generated option entities are resolved
    // by Player's modal state machine.  The lifecycle tag keeps the
    // authoritative ownership explicit without attaching a second Power
    // graph (which would execute the branch twice).
    CHOOSE_ONE_SOURCE,
    CHOOSE_ONE_OPTION,
};
//!
//! \brief CardDef class.
//!
//! This class stores the card data such as powers and play requirements.
//!
class CardDef
{
 public:
    //! Default constructor.
    CardDef() = default;

    //! Constructs card def with given \p _power.
    //! \param _power The power data.
    explicit CardDef(Power _power);

    //! Constructs a definition resolved by the named simulator lifecycle.
    explicit CardDef(CardLifecycle lifecycle) : lifecycle(lifecycle) {}

    //! Constructs card def with given \p _power and \p _playReqs.
    //! \param _power The power data.
    //! \param _playReqs The play requirements data.
    explicit CardDef(Power _power, std::map<PlayReq, int> _playReqs);

    //! Returns whether this definition supplies an executable Battlecry.
    bool HasBattlecry() const noexcept
    {
        return !power.GetBattlecryTask().empty();
    }

    Power power;
    std::map<PlayReq, int> playReqs;
    CardLifecycle lifecycle = CardLifecycle::NONE;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_CARD_DEF_HPP
