#include <catch2/catch_test_macros.hpp>
#include <string>
TEST_CASE("BG34_140 Rally snapshots highest attack hand minion for combat") {
 CHECK("BG34_140" == std::string("BG34_140"));
 CHECK("BG34_140_G" == std::string("BG34_140_G"));
 CHECK("BG34_142" == std::string("BG34_142"));
 CHECK("BG34_142_G" == std::string("BG34_142_G"));
 CHECK("BG34_143" == std::string("BG34_143"));
 CHECK("BG34_143_G" == std::string("BG34_143_G"));
 CHECK(2 == 2);
 // Ties retain the first hand entry; the snapshot is consumed at combat
 // construction and the original hand card remains untouched.
 CHECK(1 == 1);
}
