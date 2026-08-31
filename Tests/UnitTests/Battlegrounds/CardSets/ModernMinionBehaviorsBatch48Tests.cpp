#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch48.hpp>
#include <map>
#include <string>
TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 48 tempest") { std::map<std::string,RosettaStone::Battlegrounds::CardDef> cards; RosettaStone::Battlegrounds::ModernMinionBehaviorsBatch48::AddAll(cards); CHECK(cards.contains("BG36_352")); CHECK(cards.contains("BG36_352_G")); }
