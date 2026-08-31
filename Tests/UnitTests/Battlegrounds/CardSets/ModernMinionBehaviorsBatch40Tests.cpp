#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch40.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentBeetleBuffTask.hpp>

#include <map>
#include <string>
#include <variant>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 40 scorpid")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch40::AddAll(cards);

    CHECK_EQ(cards.size(), 2);
    CHECK_EQ(cards.at("BG36_209").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG36_209_G").power.GetDeathrattleTask().size(), 1);
    REQUIRE(cards.at("BG36_209").power.GetTrigger().has_value());
    REQUIRE(cards.at("BG36_209_G").power.GetTrigger().has_value());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 40 golden scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch40::AddAll(cards);

    const auto* normal = std::get_if<SimpleTasks::PersistentBeetleBuffTask>(
        &cards.at("BG36_209").power.GetTrigger()->GetTasks().front());
    const auto* golden = std::get_if<SimpleTasks::PersistentBeetleBuffTask>(
        &cards.at("BG36_209_G").power.GetTrigger()->GetTasks().front());
    REQUIRE(normal != nullptr);
    REQUIRE(golden != nullptr);
    CHECK_EQ(normal->GetAttack(), 5);
    CHECK_EQ(normal->GetHealth(), 5);
    CHECK_EQ(golden->GetAttack(), 10);
    CHECK_EQ(golden->GetHealth(), 10);
}
