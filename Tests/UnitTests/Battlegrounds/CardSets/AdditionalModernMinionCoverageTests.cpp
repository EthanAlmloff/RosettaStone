#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch59.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch63.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch65.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds] - late modern minion registries expose exact pool rows")
{
    std::map<std::string, CardDef> batch59;
    ModernMinionBehaviorsBatch59::AddAll(batch59);
    for (const auto id : {"BG24_715", "BG24_715_G"}) {
        REQUIRE(batch59.contains(id));
        CHECK(batch59.at(id).power.GetTrigger().has_value());
        CHECK(batch59.at(id).power.GetTrigger()->GetTriggerType() ==
              TriggerType::SELL_MINION);
    }

    std::map<std::string, CardDef> batch63;
    ModernMinionBehaviorsBatch63::AddAll(batch63);
    for (const auto id : {"BG23_318", "BG23_318_G"}) {
        REQUIRE(batch63.contains(id));
        CHECK(batch63.at(id).power.GetDeathrattleTask().size() == 1);
    }

    std::map<std::string, CardDef> batch65;
    ModernMinionBehaviorsBatch65::AddAll(batch65);
    for (const auto id : {"BG24_018", "BG24_018_G"}) {
        REQUIRE(batch65.contains(id));
        // Static keyword behavior is owned by card metadata; the empty Power
        // is intentional and still required for pool registration.
        CHECK(batch65.at(id).power.GetBattlecryTask().empty());
        CHECK(batch65.at(id).power.GetDeathrattleTask().empty());
    }

    std::map<std::string, CardDef> modern;
    ModernMinionBehaviors::AddAll(modern);
    for (const auto id : {"BG27_513_G", "BG32_873", "BG32_873_G",
                          "BG34_322", "BG34_322_G"}) {
        REQUIRE(modern.contains(id));
        CHECK(modern.at(id).power.GetBattlecryTask().empty());
        CHECK(modern.at(id).power.GetDeathrattleTask().empty());
    }
}
