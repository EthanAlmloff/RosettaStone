#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds] - Ancient Wishbone is an executor multiplier marker")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_804").effect ==
          TrinketEffect::HERO_POWER_TWICE);
}
