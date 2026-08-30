#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch35.hpp>
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch35] - Meteorite Crasher after-sell Elemental trigger") {
    std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch35::AddAll(cards); GeneratedBehaviorMappings::AddAll(cards);
    for (const auto* id : {"BG31_843", "BG31_843_G"}) {
        REQUIRE(cards.contains(id)); REQUIRE(cards.at(id).power.GetTrigger().has_value());
        CHECK(cards.at(id).power.GetTrigger()->GetTriggerType() == TriggerType::SELL_MINION);
        CHECK(cards.at(id).power.GetTrigger()->GetTriggerSource() == TriggerSource::FRIENDLY);
        REQUIRE(cards.at(id).power.GetTrigger()->GetTasks().size() == 1);
        CHECK(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(
            cards.at(id).power.GetTrigger()->GetTasks().front()));
    }
}
