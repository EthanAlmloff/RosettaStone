#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch50.hpp>
#include <map>
#include <string>
TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 50 mastermind") { std::map<std::string,RosettaStone::Battlegrounds::CardDef> cards; RosettaStone::Battlegrounds::ModernMinionBehaviorsBatch50::AddAll(cards); CHECK(cards.contains("BG36_507")); CHECK(cards.contains("BG36_507_G")); CHECK(cards.at("BG36_507").power.GetActivate().has_value()); CHECK(cards.at("BG36_507_G").power.GetActivate()->amount==2); }
