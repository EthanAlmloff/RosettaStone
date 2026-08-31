#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch47.hpp>
#include <map>
#include <string>
TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 47 lockbox") { std::map<std::string,RosettaStone::Battlegrounds::CardDef> cards; RosettaStone::Battlegrounds::ModernMinionBehaviorsBatch47::AddAll(cards); CHECK(cards.contains("BG36_523")); CHECK(cards.contains("BG36_523_G")); }
