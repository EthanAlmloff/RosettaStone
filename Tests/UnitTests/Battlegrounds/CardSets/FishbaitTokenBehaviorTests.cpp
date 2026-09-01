// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/FishbaitBehaviors.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Fishbait] - generated token deathrattle registry")
{
    CHECK(FISHBAIT_SELL_BEHAVIORS.size() == 4);
    CHECK(FISHBAIT_TOKEN_BEHAVIORS.size() == 2);
    CHECK(FindFishbaitSellBehavior(133455)->id == "BG36_181");
    CHECK(FindFishbaitSellBehavior(133456)->id == "BG36_181_G");
    CHECK(FindFishbaitSellBehavior(132804)->id == "BG36_206");
    CHECK(FindFishbaitSellBehavior(132805)->id == "BG36_206_G");
    CHECK(FindFishbaitSellBehavior(133455)->kind == FishbaitSellKind::AIR_BALLER);
    CHECK(FindFishbaitSellBehavior(133456)->kind == FishbaitSellKind::AIR_BALLER);
    constexpr std::array expected = {
        std::pair{"BG36_205", 132802},
        std::pair{"BG36_205_G", 132803},
    };
    for (const auto& [id, dbfID] : expected)
    {
        const auto* definition = FindFishbaitTokenBehavior(dbfID);
        REQUIRE(definition != nullptr);
        CHECK(definition->id == id);
        CHECK(definition->dbfID == dbfID);
        CHECK(definition->killerStat == (dbfID == 132803 ? 10 : 5));
    }
    CHECK(FindFishbaitTokenBehavior(0) == nullptr);
}
