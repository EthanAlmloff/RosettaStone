#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch56.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch56] - registered variants")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch56::AddAll(cards);
    REQUIRE(cards.size() == 8);
    CHECK(cards.contains("BG21_015"));
    CHECK(cards.contains("BG21_015_G"));
    CHECK(cards.contains("BG23_009"));
    CHECK(cards.contains("BG23_009_G"));
    CHECK(cards.contains("BGS_115"));
    CHECK(cards.contains("TB_BaconUps_156"));
    CHECK(cards.at("BGS_115").power.GetTrigger()->GetTriggerType() ==
          TriggerType::SELL_MINION);
    CHECK(cards.at("TB_BaconUps_156").power.GetTrigger()->GetTriggerType() ==
          TriggerType::SELL_MINION);
}
