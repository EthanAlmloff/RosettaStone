#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch25.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("Batch25 registers only exact targeted and friendly-board effects")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch25::AddAll(cards);
    for (const auto* id : { "BG25_004", "BG25_004_G", "BG26_522", "BG26_522_G",
                            "BG30_756", "BG30_756_G", "BG32_824", "BG32_824_G",
                            "BG33_701", "BG33_701_G", "BG27_002", "BG27_002_G",
                            "BG_AT_069", "BG_AT_069_G" })
        CHECK(cards.contains(id));
    CHECK(cards.at("BG25_004").playReqs.contains(PlayReq::REQ_TARGET_WITH_RACE));
    CHECK(cards.at("BG30_756").playReqs.contains(PlayReq::REQ_TARGET_WITH_RACE));
    CHECK(cards.at("BG32_824").power.GetBattlecryTask().size() == 1);
    CHECK(cards.at("BG32_824").power.GetDeathrattleTask().size() == 1);
    CHECK(std::holds_alternative<SimpleTasks::FriendlyRaceEnchantmentTask>(
        cards.at("BG32_824").power.GetDeathrattleTask().front()));
    CHECK(cards.at("BG33_701").power.GetRallyTask().size() == 1);
    CHECK(cards.at("BG_AT_069").playReqs.contains(PlayReq::REQ_MINION_TARGET));
}
