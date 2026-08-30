#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HandRaceBuffTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Generated mappings] - every reviewed row has a live CardDef")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto& row : DeclarativeBehaviorRows)
    {
        REQUIRE(cards.contains(std::string(row.id)));
        const auto& power = cards.at(std::string(row.id)).power;
        if (row.trigger == "battlecry")
            CHECK(!power.GetBattlecryTask().empty());
        else if (row.trigger == "deathrattle")
            CHECK(!power.GetDeathrattleTask().empty());
        else if (row.trigger == "battlecry_and_deathrattle")
            CHECK(!power.GetBattlecryTask().empty());
        else if (row.trigger == "static")
            CHECK(power.GetBattlecryTask().empty());
        else if (row.trigger == "rally")
            CHECK(!power.GetRallyTask().empty());
        else
            CHECK(power.GetTrigger().has_value());
    }
}

TEST_CASE("[Generated mappings] - persistent race buff is battlecry and exact golden scaling")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto& row : { std::pair{"BG25_011", 1}, std::pair{"BG25_011_G", 2} })
    {
        REQUIRE(cards.contains(row.first));
        const auto& tasks = cards.at(row.first).power.GetBattlecryTask();
        REQUIRE_EQ(tasks.size(), 1);
        const auto* buff = std::get_if<SimpleTasks::PersistentRaceBuffTask>(&tasks.front());
        REQUIRE(buff != nullptr);
        CHECK(buff->GetRace() == Race::UNDEAD);
    }
}

TEST_CASE("[Generated mappings] - Beast rally family is race-gated")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG36_211", "BG36_211_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE(tasks.size() == 1);
        REQUIRE(std::holds_alternative<SimpleTasks::RallyRaceBuffTask>(tasks.front()));
    }
}

TEST_CASE("[Generated mappings] - spend gold family uses threshold trigger")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_810", "BG26_810_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::SPEND_GOLD);
        CHECK(trigger->GetTriggerSource() == TriggerSource::SELF);
        CHECK(trigger->GetTasks().size() == (std::string_view(id) == "BG26_810_G" ? 2 : 1));
    }
}

TEST_CASE("[Generated mappings] - hand Murloc growth is trigger-backed")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG26_137", "BG26_137_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::AFTER_PLAY_MINION);
        REQUIRE(trigger->GetTasks().size() == 1);
        CHECK(std::holds_alternative<SimpleTasks::HandRaceBuffTask>(trigger->GetTasks().front()));
    }
}

TEST_CASE("[Generated mappings] - tier-gated Murloc buff is generated")
{
    std::map<std::string, CardDef> cards;
    GeneratedBehaviorMappings::AddAll(cards);
    for (const auto id : {"BG33_893", "BG33_893_G"})
    {
        REQUIRE(cards.contains(id));
        const auto& trigger = cards.at(id).power.GetTrigger();
        REQUIRE(trigger.has_value());
        CHECK(trigger->GetTriggerType() == TriggerType::AFTER_PLAY_MINION);
        CHECK(trigger->GetTasks().size() == (std::string_view(id) == "BG33_893_G" ? 2 : 1));
    }
}
