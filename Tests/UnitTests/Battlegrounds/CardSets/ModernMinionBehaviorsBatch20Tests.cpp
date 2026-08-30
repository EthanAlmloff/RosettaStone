#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch20.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch20] - Giant Rat economy scales")
{
    struct Expected
    {
        const char* id;
        int amount;
    };
    constexpr Expected expected[] = {
        { "BG34_Giant_001", 1 }, { "BG34_Giant_001_G", 2 }
    };
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch20::AddAll(cards);
    REQUIRE_EQ(cards.size(), 2);
    for (const auto& entry : expected)
    {
        const auto* id = entry.id;
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().size() == 1);
        CHECK(cards.at(id).power.GetDeathrattleTask().size() == 1);
        const auto* battlecry = std::get_if<SimpleTasks::GainGoldTask>(
            &cards.at(id).power.GetBattlecryTask().front());
        const auto* deathrattle = std::get_if<SimpleTasks::GainGoldTask>(
            &cards.at(id).power.GetDeathrattleTask().front());
        REQUIRE(battlecry != nullptr);
        REQUIRE(deathrattle != nullptr);
        CHECK_EQ(battlecry->Amount(), entry.amount);
        CHECK_EQ(deathrattle->Amount(), entry.amount);
        CHECK(battlecry->IsNextTurn());
        CHECK(deathrattle->IsNextTurn());
    }
}
