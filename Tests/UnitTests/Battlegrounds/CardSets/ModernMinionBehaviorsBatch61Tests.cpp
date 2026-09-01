#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch61.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/NomiElementalTavernBuffTask.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch61] - Nomi aura")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch61::AddAll(cards);
    REQUIRE(cards.size() == 2);
    CHECK(cards.contains("BGS_104"));
    CHECK(cards.contains("TB_BaconUps_201"));
    CHECK(std::get<SimpleTasks::NomiElementalTavernBuffTask>(
              cards.at("BGS_104").power.GetTrigger()->GetTasks().front())
              .Amount() == 4);
    CHECK(std::get<SimpleTasks::NomiElementalTavernBuffTask>(
              cards.at("TB_BaconUps_201")
                  .power.GetTrigger()
                  ->GetTasks()
                  .front())
              .Amount() == 8);
}
