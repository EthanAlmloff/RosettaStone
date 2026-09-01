// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;
namespace SimpleTasks { class DarkGiftRandomPoolTask; }

//! Small, explicitly verified behavior families for Patch 36.4 Dark Gifts.
//! Metadata flags alone never make a gift executable; callers must use this
//! lookup before exposing a gift to the action bridge.
enum class DarkGiftEffect
{
    NONE,
    TARGET_STATS,
    ALL_RACES,
    SUNKEN_PERSISTENCE,
    TIME_TURNING,
    TARECGOSA_BLESSING,
    STEADY_GROWTH,
    AFFINITY,
    POLARIZATION,
    TARGET_KEYWORDS,
    TARGET_MULTI_HIT_DIVINE_SHIELD,
    TARGET_GOLDEN,
    TARGET_REBORN,
    TARGET_STEALTH,
    START_COMBAT_STATS,
    START_COMBAT_DEATHRATTLE,
    START_COMBAT_LEFT_ATTACK,
    IMMUNE_WHILE_ATTACKING,
    PLAY_CARD_STATS,
    END_TURN_BATTLECRY,
    DEATHRATTLE_STATS,
    DEATHRATTLE_FREE_REFRESH,
    COUNTER_STATS,
    INCUBATION,
    RANDOM_POOL_TASK,
    RALLY_BLOOD_GEMS,
    HAND_COPY,
    REPLICATION,
    FODDER_REFRESH,
};

struct DarkGiftBehavior
{
    DarkGiftEffect effect = DarkGiftEffect::NONE;
    int attack = 0;
    int health = 0;
    bool divineShield = false;
    bool windfury = false;
    bool venomous = false;
    unsigned char uses = 1;
    bool golden = false;
    bool reborn = false;
    bool stealth = false;
    int startCombatAttackMultiplier = 1;
    int startCombatHealthMultiplier = 1;
    int playCardAttack = 0;
    int playCardHealth = 0;
    //! Counter family: 1=battlecries, 2=deathrattles, 3=Tavern spells.
    int counterKind = 0;
    //! Random-pool family: 1=most-common-race Rally, 2=Tavern-spell Deathrattle.
    int randomPoolKind = 0;
    //! Number of free Tavern refreshes granted when the source dies.
    int freeRefreshes = 0;
    int divineShieldHits = 0;
    int incubationTurns = 0;
    //! Fodders added to the next refreshes by Demonology.
    int fodderRefreshes = 0;
};

//! Returns the complete behavior for a supported gift, or NONE when the
//! entity remains intentionally fail-closed.
DarkGiftBehavior FindDarkGiftBehavior(std::string_view id);

//! Returns whether a board minion is a legal target for this behavior.
//! This predicate is shared by action-mask construction and execution so a
//! known gift never enters the bridge with an impossible target.
bool DarkGiftTargetIsLegal(const Minion& target,
                           const DarkGiftBehavior& behavior);

//! Applies a previously validated gift to one friendly board minion.
//! Returns false for an unsupported behavior and performs no mutation then.
bool ApplyDarkGift(Minion& target, const DarkGiftBehavior& behavior,
                   int currentCount);
bool ApplyDarkGift(Minion& target, const DarkGiftBehavior& behavior);

//! Applies a gift when its effect also needs the owning player's hand.
bool ApplyDarkGift(Player& player, Minion& target,
                   const DarkGiftBehavior& behavior, int currentCount = 0);

//! Registers behavior definitions so CardLoader's behavior marker and the
//! coverage tooling agree with the executable lookup above.
class DarkGiftBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_BEHAVIORS_HPP
