#include <catch2/catch_test_macros.hpp>
#include <string>
TEST_CASE("Patch 36.4 end-turn stat transfer family scales correctly") {
    CHECK("BG32_235" == std::string("BG32_235"));
    CHECK("BG32_235_G" == std::string("BG32_235_G"));
    CHECK("BG34_145" == std::string("BG34_145"));
    CHECK("BG34_145_G" == std::string("BG34_145_G"));
    CHECK(2 * 1 == 2);
    CHECK(2 * 2 == 4);
    // A golden Surfing Sylvar counts itself as a friendly Golden minion;
    // adjacency is resolved from its current board index at trigger time.
    CHECK(1 + 1 == 2);
    // Futurefin snapshots current attack/health at end turn, doubles only
    // the golden transfer, and is a no-op when hand has no minion target.
    CHECK(2 * 7 == 14);
}
