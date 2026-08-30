#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsSimpleBatch.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>

#include <doctest/doctest.h>
#include <map>
#include <string>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[ModernMinionBehaviorsSimpleBatch] - registers normal and golden families")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsSimpleBatch::AddAll(cards);
    for (const auto* id : { "BG29_888", "BG29_888_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetRallyTask().size() > 0);
    }
}

TEST_CASE("[ModernMinionBehaviorsSimpleBatch] - golden doubles exact task count")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsSimpleBatch::AddAll(cards);
    CHECK(cards.at("BG29_888_G").power.GetRallyTask().size() == 2);
}

TEST_CASE("[ModernMinionBehaviorsSimpleBatch] - Rally targets self")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsSimpleBatch::AddAll(cards);
    for (const auto* id : { "BG29_888", "BG29_888_G" })
    {
        CAPTURE(id);
        const auto& tasks = cards.at(id).power.GetRallyTask();
        REQUIRE_FALSE(tasks.empty());
        for (const auto& task : tasks)
        {
            const auto* enchant =
                std::get_if<SimpleTasks::AddEnchantmentTask>(&task);
            REQUIRE(enchant != nullptr);
            CHECK(enchant->CardID() == "BG29_888e");
            CHECK(enchant->Entity() == EntityType::SOURCE);
            CHECK_FALSE(enchant->UsesScriptTag());
        }
    }
}
