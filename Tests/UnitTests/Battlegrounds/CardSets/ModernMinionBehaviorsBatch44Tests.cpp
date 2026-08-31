#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch44.hpp>
#include <map>
#include <string>
TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 44 butcher") {
 std::map<std::string, RosettaStone::Battlegrounds::CardDef> cards;
 RosettaStone::Battlegrounds::ModernMinionBehaviorsBatch44::AddAll(cards);
 REQUIRE(cards.contains("BG32_324")); REQUIRE(cards.contains("BG32_324_G"));
 CHECK(cards.at("BG32_324").power.GetAvenge().has_value());
 CHECK(cards.at("BG32_324_G").power.GetAvenge().has_value());
}
