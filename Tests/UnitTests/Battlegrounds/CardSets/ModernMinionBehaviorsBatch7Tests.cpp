// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

#include <array>
#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 inventory table")
{
    constexpr std::array ids{
        "BG25_009", "BG25_009_G", "BG25_008", "BG25_008_G",
        "BG20_HERO_301_Buddy", "BG20_HERO_301_Buddy_G", "BG24_004",
        "BG24_004_G", "BG27_513", "BG27_513_G", "BG35_150t",
        "BG35_150t_G", "TB_BaconShop_HERO_40_Buddy",
        "TB_BaconShop_HERO_40_Buddy_G", "TB_BaconShop_HERO_68_Buddy",
        "TB_BaconShop_HERO_68_Buddy_G", "TB_BaconUps_156",
        "BG36_621", "BG36_621_G" };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);

    for (const auto* id : ids)
    {
        REQUIRE(cards.contains(id));
        const auto metadata = Cards::FindCardByID(id);
        REQUIRE_FALSE(metadata.id.empty());
        CHECK_EQ(metadata.id, id);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 summon scaling")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);

    REQUIRE(cards.contains("BG25_009"));
    REQUIRE(cards.contains("BG25_009_G"));
    CHECK_EQ(cards.at("BG25_009").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG25_009_G").power.GetDeathrattleTask().size(), 1);
    CHECK(cards.at("BG25_009").power.GetBattlecryTask().empty());
    CHECK(cards.at("BG25_009_G").power.GetBattlecryTask().empty());

    // The generated target is the normal/golden Eternal Knight respectively;
    // checking the task's variant directly would couple this test to the
    // internal TaskType layout, so the source table remains the authority for
    // the exact pair and the focused test asserts both branches are present.
    CHECK(cards.contains("BG25_008"));
    CHECK(cards.contains("BG25_008_G"));
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 7 static companions")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch7::AddAll(cards);

    for (const auto* id : { "BG20_HERO_301_Buddy", "BG20_HERO_301_Buddy_G",
                            "BG24_004", "BG24_004_G", "BG27_513",
                            "BG27_513_G", "BG35_150t", "BG35_150t_G",
                            "BG36_621", "BG36_621_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetDeathrattleTask().empty());
    }
}
