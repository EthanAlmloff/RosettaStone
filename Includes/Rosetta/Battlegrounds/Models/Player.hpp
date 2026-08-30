// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP

#include <Rosetta/Battlegrounds/Models/Hero.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <Rosetta/Battlegrounds/Models/Tavern.hpp>
#include <Rosetta/Battlegrounds/Tasks/TaskStack.hpp>
#include <Rosetta/Battlegrounds/Zones/FieldZone.hpp>
#include <Rosetta/Battlegrounds/Zones/HandZone.hpp>

#include <array>
#include <functional>
#include <limits>

namespace RosettaStone::Battlegrounds
{
class Battle;

//!
//! \brief Player class.
//!
//! This class stores various information that used in Battlegrounds.
//!
class Player
{
 public:
    //! Returns the field according the status.
    //! \return The field according the status.
    FieldZone& GetField();

    //! Initializes a Hero instance.
    //! \param idx The index of hero choices.
    void SelectHero(std::size_t idx);

    //! Prepare a list of minions in Tavern for purchase.
    void PrepareTavern();

    //! Applies passive modifiers that belong to a newly created minion
    //! instance.  This is intentionally callable by pool and summon paths so
    //! passive hero powers do not depend on the minion first appearing in the
    //! Tavern.
    void ApplyFreshMinionModifiers(Minion& minion) const;

    //! Purchases a minion from Tavern's field.
    //! \param idx The index of a list of minions in Tavern's field.
    void PurchaseMinion(std::size_t idx);

    //! Plays a minion or spell card.
    //! \param handIdx The index of a list of cards in player's hand.
    //! \param fieldIdx The index of player's field to add.
    //! \param targetIdx The index of the target in player's field.
    void PlayCard(std::size_t handIdx, std::size_t fieldIdx,
                  int targetIdx = -1);

    //! Returns whether a supported no-target Tavern spell can be played.
    //! \param handIdx The index of the spell in the player's hand.
    bool CanPlaySpell(std::size_t handIdx) const;

    //! Returns whether a supported Tavern spell can be played on a friendly
    //! board target. A target of -1 denotes a no-target spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \param targetIdx The friendly board slot, or -1 for no target.
    bool CanPlaySpell(std::size_t handIdx, int targetIdx) const;

    //! Pays for and resolves a supported no-target Tavern spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \return false when the card, cost, or behavior is unsupported.
    bool PlaySpell(std::size_t handIdx);

    //! Pays for and resolves a supported targeted or no-target Tavern spell.
    //! \param handIdx The index of the spell in the player's hand.
    //! \param targetIdx The friendly board slot, or -1 for no target.
    //! \return false when the card, target, cost, or behavior is unsupported.
    bool PlaySpell(std::size_t handIdx, int targetIdx);

    //! Applies a fully resolved target-free Season 14 hero-power activation.
    //! Random recipient selection uses RosettaStone's shared RNG stream.
    bool ApplySeason14HeroPowerBatch3Activation(
        const Season14HeroPowerBatch3Activation& activation);

    //! Sells a minion to Tavern.
    //! \param idx The index of a list of minions in player's field.
    void SellMinion(std::size_t idx);

    //! Upgrades your Tavern to the next tier.
    void UpgradeTavern();

    //! Refreshes a list of minions in Tavern's field.
    //! \p freeRefresh is used by a hero power whose activation already paid
    //! for the refresh (for example Temporal Tavern).
    void RefreshTavern(bool freeRefresh = false);

    //! Freezes a list of minions in Tavern's field.
    void FreezeTavern();

    //! Rearranges a minion to another position on player's field.
    //! \param curIdx The current index of minion.
    //! \param newIdx The new index of minion.
    void RearrangeMinion(std::size_t curIdx, std::size_t newIdx);

    //! Completes recruit phase.
    void CompleteRecruit() const;

    //! Processes the tasks related to defeat.
    void ProcessDefeat();

    PlayState playState = PlayState::INVALID;
    std::size_t idx = 0;
    std::size_t rank = 1;

    Hero hero;

    int remainCoin = 0;
    int totalCoin = 0;
    int armor = 0;
    int currentTier = 0;
    int coinToUpgradeTavern = 0;

    Tavern tavern;
    HandZone hand;
    FieldZone recruitField;
    FieldZone battleField;

    TaskStack taskStack;
    Season14State season14;

    std::function<void(Player&)> selectHeroCallback;
    std::function<void(Player&)> prepareTavernMinionsCallback;
    std::function<void(Player&, std::size_t)> purchaseMinionCallback;
    std::function<int()> getNextCardIndexCallback;
    std::function<void(int)> returnMinionCallback;
    std::function<void(Player&)> clearTavernMinionsCallback;
    std::function<void(Player&)> upgradeTavernCallback;
    std::function<void()> completeRecruitCallback;
    std::function<Player&(Player&)> getOpponentPlayerCallback;
    std::function<Battle&()> getBattleCallback;
    std::function<void(Player&)> processDefeatCallback;

    std::array<int, 4> heroChoices{ 0, 0, 0, 0 };

    std::size_t playerIdxNextFight = std::numeric_limits<std::size_t>::max();
    std::size_t playerIdxFoughtLastTurn =
        std::numeric_limits<std::size_t>::max();

    bool isInCombat = false;
    bool isFoughtGhostLastTurn = false;
    bool freezeTavern = false;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_PLAYER_HPP
