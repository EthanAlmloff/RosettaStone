#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CopyTargetBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerAdjacentBattlecryTask.hpp>
#include <map>
#include <string_view>
#include <variant>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch74] Rylak triggers both adjacent Battlecries with golden repeats")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);
    for (const auto* id : {"BG26_801", "BG26_801_G"}) {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetDeathrattleTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::TriggerAdjacentBattlecryTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->Golden() == (std::string_view(id) == "BG26_801_G"));
    }
}

TEST_CASE("[Batch74] Faceless requires a non-self friendly minion and goldenizes")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviors::AddAll(cards);
    for (const auto* id : {"BG_EX1_564", "BG_EX1_564_G"}) {
        REQUIRE(cards.contains(id));
        const auto& def = cards.at(id);
        CHECK(def.playReqs.contains(PlayReq::REQ_TARGET_TO_PLAY));
        CHECK(def.playReqs.contains(PlayReq::REQ_MINION_TARGET));
        CHECK(def.playReqs.contains(PlayReq::REQ_FRIENDLY_TARGET));
        CHECK(def.playReqs.contains(PlayReq::REQ_NONSELF_TARGET));
        const auto& tasks = def.power.GetBattlecryTask();
        REQUIRE(tasks.size() == 1);
        const auto* task = std::get_if<SimpleTasks::CopyTargetBattlecryTask>(&tasks.front());
        REQUIRE(task != nullptr);
        CHECK(task->Golden() == (std::string_view(id) == "BG_EX1_564_G"));
    }
}
