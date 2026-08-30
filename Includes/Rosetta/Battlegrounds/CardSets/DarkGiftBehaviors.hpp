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

//! Small, explicitly verified behavior families for Patch 36.4 Dark Gifts.
//! Metadata flags alone never make a gift executable; callers must use this
//! lookup before exposing a gift to the action bridge.
enum class DarkGiftEffect
{
    NONE,
    TARGET_STATS,
    TARGET_KEYWORDS,
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
};

//! Returns the complete behavior for a supported gift, or NONE when the
//! entity remains intentionally fail-closed.
DarkGiftBehavior FindDarkGiftBehavior(std::string_view id);

//! Applies a previously validated gift to one friendly board minion.
//! Returns false for an unsupported behavior and performs no mutation then.
bool ApplyDarkGift(Minion& target, const DarkGiftBehavior& behavior);

//! Registers behavior definitions so CardLoader's behavior marker and the
//! coverage tooling agree with the executable lookup above.
class DarkGiftBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_DARK_GIFT_BEHAVIORS_HPP
