#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch13.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>

#include <map>
#include <string>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[ModernMinionBehaviorsBatch13] - exact gold Battlecries")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch13::AddAll(cards);

    struct Expected
    {
        const char* id;
        int amount;
        bool nextTurn;
    };
    constexpr Expected expected[] = {
        { "BG23_002", 1, false },
        { "BG23_002_G", 2, false },
        { "BG26_135", 1, true },
        { "BG26_135_G", 2, true },
    };
    CHECK_EQ(cards.size(), 4);
    for (const auto& entry : expected)
    {
        CAPTURE(entry.id);
        REQUIRE(cards.contains(entry.id));
        const auto& tasks = cards.at(entry.id).power.GetBattlecryTask();
        REQUIRE_EQ(tasks.size(), 1);
        const auto* gold = std::get_if<SimpleTasks::GainGoldTask>(&tasks.front());
        REQUIRE(gold != nullptr);
        CHECK_EQ(gold->Amount(), entry.amount);
        CHECK_EQ(gold->IsNextTurn(), entry.nextTurn);
    }
}
