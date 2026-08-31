#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch54.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch54] - Gatekeeper variants") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch54::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.contains("BG36_640"));
    CHECK(cards.contains("BG36_640_G"));
    CHECK(cards.at("BG36_640").power.GetTrigger()->GetTriggerType() == TriggerType::AFTER_CAST_SPELL);
    CHECK(cards.at("BG36_640_G").power.GetTrigger()->GetTriggerType() == TriggerType::AFTER_CAST_SPELL);
    CHECK(cards.at("BG36_640").power.GetTrigger()->GetTriggerSource() == TriggerSource::SELF);
    CHECK(cards.at("BG36_640_G").power.GetTrigger()->GetTriggerSource() == TriggerSource::SELF);
    const auto& normal = std::get<SimpleTasks::CastTavernSpellTask>(cards.at("BG36_640").power.GetTrigger()->GetTasks().front());
    const auto& golden = std::get<SimpleTasks::CastTavernSpellTask>(cards.at("BG36_640_G").power.GetTrigger()->GetTasks().front());
    CHECK(normal.CardID() == "BG28_888");
    CHECK(normal.Amount() == 1);
    CHECK(golden.Amount() == 2);
}
