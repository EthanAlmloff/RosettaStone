// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace RosettaStone::Battlegrounds
{
//! Public modal decisions introduced by modern Battlegrounds content.
enum class Season14Decision : std::int32_t
{
    NONE = 0,
    CHOICE = 2,
    DISCOVER = 3,
    TRINKET_SELECTION = 4,
    DARK_GIFT_SELECTION = 5
};

//! Events at which modern effects may be activated.
enum class Season14Event : std::uint8_t
{
    RECRUIT_START,
    RECRUIT_END,
    COMBAT_START,
    COMBAT_END,
    SPELL_CAST,
    COUNT
};

//! A public offering. Hidden simulator state must never be put in this list.
struct Season14Offering
{
    std::int32_t dbfID = 0;
    std::uint64_t entityID = 0;
};

//! Persistent Trinket or Dark Gift state owned by one player.
struct Season14PersistentEffect
{
    std::int32_t dbfID = 0;
    std::uint8_t remainingUses = 0;
    bool active = true;
};

inline constexpr std::size_t SEASON14_TRINKET_SLOTS = 2;
inline constexpr std::size_t SEASON14_DARK_GIFT_SLOTS = 16;
//! Small, simulator-independent state machine for modern modal mechanics.
class Season14State
{
 public:
    Season14Decision pendingDecision = Season14Decision::NONE;
    std::vector<Season14Offering> choiceOfferings;
    std::vector<Season14Offering> pendingOfferings;
    std::vector<Season14PersistentEffect> trinkets;
    std::vector<Season14PersistentEffect> darkGifts;

    std::int32_t heroPowerDbfID = 0;
    std::int32_t heroPowerCost = 0;
    bool heroPowerAvailable = false;
    bool heroPowerUsed = false;

    //! Hooks for effects whose entity behavior is implemented elsewhere.
    bool lockboxActive = false;
    bool fishbaitActive = false;
    std::array<std::uint64_t, static_cast<std::size_t>(
                                  Season14Event::COUNT)>
        eventCounts{};

    //! Replaces the current public modal offering and enters its decision.
    void BeginDecision(Season14Decision decision,
                       std::vector<Season14Offering> offerings);

    //! Selects one public offering and clears the pending decision.
    //! \return false when no matching pending offering exists.
    bool SelectDecision(std::size_t offeringIndex);

    //! Configures the hero power without claiming its behavior is supported.
    void SetHeroPower(std::int32_t dbfID, std::int32_t cost, bool available);

    //! Returns whether the player can pay and use the power this turn.
    bool CanUseHeroPower(std::int32_t availableGold) const;

    //! Consumes the power for this turn.
    bool UseHeroPower();

    //! Returns whether another Trinket fits in the persistent slots.
    bool CanAddTrinket() const;

    //! Returns whether another Dark Gift fits in the persistent slots.
    bool CanAddDarkGift() const;

    //! Adds a persistent effect to a player-visible slot.
    void AddTrinket(Season14PersistentEffect effect);
    void AddDarkGift(Season14PersistentEffect effect);

    //! Decrements a finite effect, removing it when it reaches zero.
    bool ConsumeEffect(std::vector<Season14PersistentEffect>& effects,
                       std::size_t slot);

    //! Records an event for effect scheduling/diagnostics.
    void Emit(Season14Event event);

    //! Generic index validation used by bridge target masks.
    static bool IsValidBoardTarget(std::int32_t index,
                                   std::int32_t boardCount);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_SEASON14_HPP
