#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch28.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRaceBuffTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Batch28] - fixed Rally buff pairs") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch28::AddAll(cards);
  CHECK_EQ(cards.size(), 4);
  for (const auto id : {"BG33_247", "BG33_247_G", "BG24_708", "BG24_708_G"}) {
    REQUIRE(cards.contains(id)); REQUIRE_EQ(cards.at(id).power.GetRallyTask().size(), 1);
  }
  CHECK(std::holds_alternative<SimpleTasks::RallyBuffTask>(cards.at("BG33_247").power.GetRallyTask().front()));
  CHECK(std::holds_alternative<SimpleTasks::RallyRaceBuffTask>(cards.at("BG24_708").power.GetRallyTask().front()));
}
