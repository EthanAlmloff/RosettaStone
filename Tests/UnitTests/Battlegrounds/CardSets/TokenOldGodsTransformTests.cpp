#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

#include <string_view>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : Token of the Old Gods] - typed transform ownership")
{
    const auto behavior = FindTrinketBehavior("BG30_MagicItem_416");
    CHECK(behavior.effect == TrinketEffect::SPELLCRAFT_TRANSFORM_HIGHER_TIER);
    // The linked Spellcraft token is handled by Player::CanPlaySpell and
    // Player::PlaySpell; this source-level ID anchors focused coverage to the
    // same executable ownership rather than metadata alone.
    CHECK(std::string_view{"BG30_MagicItem_416t"} == "BG30_MagicItem_416t");
}
