#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch34.hpp>
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch34] - Sub Scrubber post-play Mech trigger") {
    std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch34::AddAll(cards); GeneratedBehaviorMappings::AddAll(cards);
    for (const auto* id : {"BG22_HERO_200_Buddy", "BG22_HERO_200_Buddy_G"}) {
        REQUIRE(cards.contains(id));
        REQUIRE(cards.at(id).power.GetTrigger().has_value());
        CHECK(cards.at(id).power.GetTrigger()->GetTriggerType() == TriggerType::AFTER_PLAY_MINION);
    }
    CHECK(cards.at("BG22_HERO_200_Buddy_e").power.GetEnchant().has_value());
    CHECK(cards.at("BG22_HERO_200_Buddy_Ge").power.GetEnchant().has_value());
}
