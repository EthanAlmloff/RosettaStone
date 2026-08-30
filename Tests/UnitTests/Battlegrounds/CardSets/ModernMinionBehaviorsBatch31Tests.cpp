#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch31.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>

#include <map>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch31] - post-Reborn trigger families")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch31::AddAll(cards);
    for (const auto* id : {"BG36_514", "BG36_514_G", "BG36_515", "BG36_515_G",
                           "TB_BaconShop_HERO_22_Buddy", "TB_BaconShop_HERO_22_Buddy_G"})
        REQUIRE(cards.contains(id));
    CHECK(cards.at("BG36_514").power.GetTrigger()->GetTriggerType() == TriggerType::REBORN);
    CHECK(cards.at("BG36_515_G").power.GetTrigger()->GetTasks().size() == 1);
    for (const auto* id : {"BG25_016", "BG27_084", "BG36_511", "BG_DEEP_015", "BG34_Giant_031"})
        CHECK_FALSE(cards.contains(id));
}
