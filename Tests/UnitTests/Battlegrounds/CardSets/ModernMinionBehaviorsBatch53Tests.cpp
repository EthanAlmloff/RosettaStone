#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch53.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizeSatelliteTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch53] - Sky-hatch Runaway Activate") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch53::AddAll(cards);
    REQUIRE(cards.size() == 12);
    CHECK(cards.at("BG36_243").power.GetActivate()->effect == ActivateEffect::TRIGGER_RALLY);
    CHECK(cards.at("BG36_243").power.GetActivate()->cost == 1);
    CHECK(cards.at("BG36_243").power.GetActivate()->amount == 1);
    CHECK(cards.at("BG36_243_G").power.GetActivate()->effect == ActivateEffect::TRIGGER_RALLY);
    CHECK(cards.at("BG36_243_G").power.GetActivate()->amount == 2);
    CHECK(cards.at("BG36_331").power.GetRallyTask().size() == 1);
    CHECK(cards.at("BG36_331_G").power.GetRallyTask().size() == 1);
    CHECK(std::get<SimpleTasks::RandomChooseOneCardToHandTask>(cards.at("BG36_331_G").power.GetRallyTask().front()).GetAmount() == 2);
    const auto& dragonbreath = std::get<SimpleTasks::CastTavernSpellTask>(cards.at("BG36_241").power.GetRallyTask().front());
    CHECK(dragonbreath.CardID() == "BG36_246");
    CHECK(dragonbreath.Amount() == 1);
    CHECK(std::get<SimpleTasks::CastTavernSpellTask>(cards.at("BG36_241_G").power.GetRallyTask().front()).Amount() == 2);
    CHECK(cards.at("BG36_853").power.GetTrigger()->GetTriggerType() == TriggerType::AFTER_CAST_SPELL);
    CHECK(std::get<SimpleTasks::MagnetizeSatelliteTask>(cards.at("BG36_853").power.GetTrigger()->GetTasks().front()).Attack() == 4);
    CHECK(std::get<SimpleTasks::MagnetizeSatelliteTask>(cards.at("BG36_851").power.GetTrigger()->GetTasks().front()).Increment() == 2);
    CHECK(std::get<SimpleTasks::MagnetizeSatelliteTask>(cards.at("BG36_851_G").power.GetTrigger()->GetTasks().front()).Attack() == 4);
    CHECK(std::get<SimpleTasks::MagnetizeSatelliteTask>(cards.at("BG36_853_G").power.GetTrigger()->GetTasks().front()).Repeats() == 2);
    CHECK(cards.at("BG36_506").power.GetActivate()->effect == ActivateEffect::ARM_MAGNETIZATION);
    CHECK(cards.at("BG36_506_G").power.GetActivate()->cost == 1);
}
