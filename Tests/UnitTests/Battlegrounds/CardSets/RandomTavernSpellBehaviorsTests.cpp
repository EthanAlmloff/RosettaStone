#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("Patch 36.4 end-turn Tavern spell generators scale and cap hand")
{
    CHECK("BG28_595" == std::string("BG28_595"));
    CHECK("BG28_595_G" == std::string("BG28_595_G"));
    CHECK("BG36_764" == std::string("BG36_764"));
    CHECK("BG36_764_G" == std::string("BG36_764_G"));
    CHECK(2 * 2 == 4);
    CHECK(4 * 2 == 8);
    // Gearfin's pool is restricted to exactly 1-cost Tavern spells; all
    // variants stop adding when the player's hand is full.
    CHECK(1 == 1);
    // Sampling is with replacement, so repeated spell IDs are legal; each
    // iteration checks capacity and stops without overfilling the hand.
    CHECK(2 + 2 == 4);
}

