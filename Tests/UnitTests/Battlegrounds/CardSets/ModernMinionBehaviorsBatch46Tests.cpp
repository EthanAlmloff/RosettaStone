#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch46.hpp>
#include <map>
#include <string>
TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 46 extortionist") { std::map<std::string,RosettaStone::Battlegrounds::CardDef> cards; RosettaStone::Battlegrounds::ModernMinionBehaviorsBatch46::AddAll(cards); CHECK(cards.contains("BG36_524")); CHECK(cards.contains("BG36_524_G")); }
