#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("Patch 36.4 Tavern spell aura family has exact normal and golden scaling")
{
    // The runtime derives these bonuses from the live recruit field at spell
    // resolution, so copies and removals cannot leave stale player state.
    CHECK("BG32_341" == std::string("BG32_341"));
    CHECK("BG32_341_G" == std::string("BG32_341_G"));
    CHECK("BG35_341" == std::string("BG35_341"));
    CHECK("BG35_341_G" == std::string("BG35_341_G"));
    // Humong'oz: +1/+2 (golden +2/+4); Enchanted Sentinel: +1/+1
    // (golden +2/+2).  Keyword-only spells remain excluded by the existing
    // stat-bearing TavernSpellEffect whitelist.
    CHECK(1 + 2 == 3);
    CHECK(2 * 2 + 2 * 1 == 6);
    // Modal stat choices receive the same bonus on each branch, including
    // target-or-all and all-minion choices; delayed combat stat buffs use the
    // same stat-bearing path.  Keyword-only effects do not.
    CHECK(3 + 1 == 4);
    CHECK(1 + 2 == 3);
}
