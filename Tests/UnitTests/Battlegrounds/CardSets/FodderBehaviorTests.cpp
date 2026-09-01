// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/FodderBehaviors.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Fodder] - generated token and Defiler registry")
{
    CHECK(FODDER_BEHAVIORS.size() == 4);
    constexpr std::array expected = {
        std::pair{"BG35_150t", 130084},
        std::pair{"BG35_150t_G", 130085},
        std::pair{"BG35_151", 130076},
        std::pair{"BG35_151_G", 130081},
    };
    for (const auto& [id, dbfID] : expected)
    {
        const auto* definition = FindFodderBehaviorByDbfID(dbfID);
        REQUIRE(definition != nullptr);
        CHECK(definition->id == id);
        CHECK(definition->dbfID == dbfID);
    }
    CHECK(FindFodderBehaviorByDbfID(0) == nullptr);
}

TEST_CASE("[Fodder] - Defiler arms the shared refresh window")
{
    const auto* normal = FindFodderBehavior("BG35_151");
    const auto* golden = FindFodderBehavior("BG35_151_G");
    REQUIRE(normal != nullptr);
    REQUIRE(golden != nullptr);
    CHECK(normal->foddersPerRefresh == 1);
    CHECK(golden->foddersPerRefresh == 2);
}
