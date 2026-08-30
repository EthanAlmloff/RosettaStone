#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatchBaller.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Baller] - exact registration")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatchBaller::AddAll(cards);
    CHECK_EQ(cards.size(), 4);
    CHECK(cards.contains("BG31_816"));
    CHECK(cards.contains("BG31_816_G"));
    CHECK(cards.contains("BG31_818"));
    CHECK(cards.contains("BG31_818_G"));
}

TEST_CASE("[Baller] - future bonus is idempotent across fresh-instance hooks")
{
    Minion fire{ Cards::FindCardByID("BG31_816") };
    CHECK_EQ(fire.GetAttack(), 4);
    CHECK_EQ(fire.GetHealth(), 3);
    fire.ApplyFutureBallerStats(2, 0);
    fire.ApplyFutureBallerStats(2, 0);
    CHECK_EQ(fire.GetAttack(), 6);
    CHECK_EQ(fire.GetHealth(), 3);

    Minion snow{ Cards::FindCardByID("BG31_818_G") };
    snow.ApplyFutureBallerStats(0, 4);
    snow.ApplyFutureBallerStats(0, 4);
    CHECK_EQ(snow.GetAttack(), 6);
    CHECK_EQ(snow.GetHealth(), 12);
}

TEST_CASE("[Baller] - future improvement accumulates with golden scaling")
{
    Season14State state;
    CHECK(state.FutureBallerStats() == std::pair{ 0, 0 });
    state.ImproveFutureBallers(1, 0);
    state.ImproveFutureBallers(0, 2);
    state.ImproveFutureBallers(2, 2);
    CHECK(state.FutureBallerStats() == std::pair{ 3, 4 });
}
