#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch23.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch23] - fixed deathrattle summon pairs")
{
    struct Expected { const char* id; const char* token; int count; };
    constexpr Expected rows[] = {
        { "BG21_029", "BG_BRM_006t", 2 },
        { "BG21_029_G", "BG_BRM_006t", 4 },
        { "BG31_807", "BG28_603t", 3 },
        { "BG31_807_G", "BG28_603t", 6 },
        { "BG33_157", "BG_CS2_065", 2 },
        { "BG33_157_G", "BG_CS2_065", 4 },
        { "BG26_ETC_321", "BG_GVG_085", 3 },
        { "BG26_ETC_321_G", "BG_GVG_085_G", 3 },
    };
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch23::AddAll(cards);
    CHECK_EQ(cards.size(), 8);
    for (const auto& row : rows)
    {
        REQUIRE(cards.contains(row.id));
        const auto& tasks = cards.at(row.id).power.GetDeathrattleTask();
        REQUIRE_EQ(tasks.size(), 1);
        REQUIRE(std::holds_alternative<SimpleTasks::SummonTask>(tasks.front()));
        const auto& summon = std::get<SimpleTasks::SummonTask>(tasks.front());
        CHECK(summon.m_cardID == row.token);
        CHECK_EQ(summon.m_amount, row.count);
    }
}
