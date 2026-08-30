#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch15.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <map>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[ModernMinionBehaviorsBatch15] - fixed deathrattle summons")
{
    struct Expected
    {
        const char* id;
        const char* token;
        int amount;
    };
    constexpr Expected expected[] = {
        { "BG26_800", "BG26_800t", 2 },
        { "BG26_800_G", "BG26_800_Gt", 2 },
        { "BG_AV_309", "BG_AV_309t", 1 },
        { "BG_AV_309_G", "BG_AV_309_Gt", 2 },
        { "BG_DMF_533", "BG_DMF_533t", 2 },
        { "TB_BaconUps_309", "TB_BaconUps_309t", 2 },
        { "BG_EX1_534", "BG_EX1_534t", 2 },
        { "TB_BaconUps_049", "TB_BaconUps_049t", 2 },
        { "BG_KAR_005", "BG_KAR_005a", 1 },
        { "TB_BaconUps_004", "TB_BaconUps_004t", 1 },
    };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch15::AddAll(cards);
    REQUIRE_EQ(cards.size(), std::size(expected));
    for (const auto& entry : expected)
    {
        CAPTURE(entry.id);
        REQUIRE(cards.contains(entry.id));
        const auto& tasks = cards.at(entry.id).power.GetDeathrattleTask();
        REQUIRE_EQ(tasks.size(), 1);
        const auto* summon =
            std::get_if<SimpleTasks::SummonTask>(&tasks.front());
        REQUIRE(summon != nullptr);
        CHECK_EQ(summon->m_cardID, entry.token);
        CHECK_EQ(summon->m_amount, entry.amount);
    }
}
