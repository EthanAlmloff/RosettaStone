#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/ActiveTribes.hpp>
#include <Rosetta/Battlegrounds/Models/MinionPool.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("Patch 36.4 has ten eligible tribes and five per lobby")
{
    for (const auto race : RosettaStone::RACES_IN_BATTLEGROUNDS)
        CHECK(IsActiveTribe(PINNED_ACTIVE_TRIBES, race));

    const auto first = SelectActiveTribes(1234);
    const auto second = SelectActiveTribes(1234);
    CHECK(first == second);
    CHECK(std::count_if(first.begin(), first.end(), [](const auto race) {
        return race != Race::INVALID;
    }) == ACTIVE_TRIBES_PER_LOBBY);
}

TEST_CASE("ordinary minion pool is filtered through active lobby tribes")
{
    static_cast<void>(Cards::GetInstance());
    MinionPool pool;
    pool.Initialize(PINNED_ACTIVE_TRIBES);
    for (const auto& minion : pool.GetMinions(1, TIER_UPPER_LIMIT, false))
        CHECK(IsActiveTribe(PINNED_ACTIVE_TRIBES, minion.GetRace()));
}
