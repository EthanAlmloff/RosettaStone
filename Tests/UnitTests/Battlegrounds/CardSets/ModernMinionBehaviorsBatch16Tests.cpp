#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch16.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <doctest/doctest.h>
#include <map>
#include <string>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch16] - Accord-o-Tron economy trigger")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch16::AddAll(cards);
    REQUIRE(cards.at("BG26_147").power.GetTrigger().has_value());
    REQUIRE(cards.at("BG26_147_G").power.GetTrigger().has_value());
    CHECK(cards.at("BG26_147").power.GetTrigger()->GetTasks().size() == 1);
    CHECK(cards.at("BG26_147_G").power.GetTrigger()->GetTasks().size() == 1);
    CHECK(cards.at("BG26_147").power.GetTrigger()->GetTriggerType() ==
          TriggerType::TURN_START);
    CHECK(cards.at("BG26_147_G").power.GetTrigger()->GetTriggerType() ==
          TriggerType::TURN_START);
    const auto* normal = std::get_if<SimpleTasks::GainGoldTask>(
        &cards.at("BG26_147").power.GetTrigger()->GetTasks().front());
    const auto* golden = std::get_if<SimpleTasks::GainGoldTask>(
        &cards.at("BG26_147_G").power.GetTrigger()->GetTasks().front());
    REQUIRE(normal != nullptr);
    REQUIRE(golden != nullptr);
    CHECK_EQ(normal->Amount(), 1);
    CHECK_EQ(golden->Amount(), 2);
    CHECK_FALSE(normal->IsNextTurn());
    CHECK_FALSE(golden->IsNextTurn());
}
