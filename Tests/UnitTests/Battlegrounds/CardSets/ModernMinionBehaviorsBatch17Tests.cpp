#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch17.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch17] - race Battlecries")
{
    constexpr const char* ids[] = { "BG29_502", "BG29_502_G", "BG_EX1_103",
        "TB_BaconUps_064", "BG_GVG_048", "TB_BaconUps_066", "BGS_053", "TB_BaconUps_138",
        "BG34_636t", "BG34_636_Gt", "BG34_637t", "BG34_637_Gt" };
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch17::AddAll(cards);
    CHECK_EQ(cards.size(), 24);
    for (const auto* id : ids)
    {
        REQUIRE(cards.contains(id));
        REQUIRE_EQ(cards.at(id).power.GetBattlecryTask().size(), 1);
        CHECK(std::holds_alternative<SimpleTasks::FriendlyRaceEnchantmentTask>(
            cards.at(id).power.GetBattlecryTask().front()));
    }
}
