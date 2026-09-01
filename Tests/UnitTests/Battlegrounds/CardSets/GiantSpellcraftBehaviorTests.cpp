// Copyright (c) 2026 Hearthstone BG AI contributors
#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/GiantSpellcraftBehaviors.hpp>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Giant spellcraft] - Glowscale and Crown pinned family")
{
    constexpr std::array ids = {"BG34_Giant_035", "BG34_Giant_035_G",
                                "BG34_Giant_035t", "BG34_Giant_035t_G"};
    CHECK(ids.size() == GIANT_SPELLCRAFT_BEHAVIORS.size());
    CHECK(GIANT_SPELLCRAFT_BEHAVIORS.size() == 4);
    CHECK(FindGiantSpellcraft(126517)->linkedMinionDbfID == 126518);
    CHECK(FindGiantSpellcraft(127386)->linkedMinionDbfID == 128719);
    CHECK(FindGiantSpellcraft(126518)->linkedMinion.empty());
    CHECK(FindGiantSpellcraft(128719)->linkedMinion.empty());
    CHECK(FindGiantSpellcraft(0) == nullptr);
}
