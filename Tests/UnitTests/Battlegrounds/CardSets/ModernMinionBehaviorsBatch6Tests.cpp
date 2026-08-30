// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch6.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

#include <array>
#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 6 inventory table")
{
    constexpr std::array ids{
        "BG31_HERO_801pt", "BG31_HERO_801pt_G", "BGS_115t",      "BGS_115t_G",
        "BG_BOT_312t",     "TB_BaconUps_032t", "BG_ICC_026t",    "BG_ICC_026t_G",
    };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch6::AddAll(cards);

    for (const auto* id : ids)
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetStartCombatTask().empty());
        CHECK(cards.at(id).power.GetDeathrattleTask().empty());

        const auto metadata = Cards::FindCardByID(id);
        REQUIRE_FALSE(metadata.id.empty());
        CHECK_EQ(metadata.id, id);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 6 preserves prior definitions")
{
    // Batch 6 must not re-register IDs owned by earlier batches.  Seed the
    // map with the earlier static families and verify that only the eight
    // genuinely new generated entities are added.
    constexpr std::array prior_ids{
        "BG25_001",       "BG25_001_G",      "BG26_175",       "BG26_175_G",
        "BG32_236",       "BG32_236_G",      "BGS_119",        "TB_BaconUps_159",
        "BGS_131",        "TB_BaconUps_251", "BG_BOT_911",     "TB_BaconUps_099",
    };

    std::map<std::string, CardDef> cards;
    for (const auto* id : prior_ids)
    {
        cards.emplace(id, CardDef{});
    }
    const auto prior_size = cards.size();
    ModernMinionBehaviorsBatch6::AddAll(cards);

    CHECK_EQ(cards.size(), prior_size + 8);
    for (const auto* id : prior_ids)
    {
        CHECK(cards.contains(id));
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 6 shared keyword metadata")
{
    for (const auto* id : { "BG25_001", "BG25_001_G" })
    {
        const auto card = Cards::FindCardByID(id);
        REQUIRE_FALSE(card.id.empty());
        CHECK(card.gameTags.contains(GameTag::TAUNT));
        CHECK(card.gameTags.contains(GameTag::REBORN));
    }

    for (const auto* id : { "BGS_119", "TB_BaconUps_159" })
    {
        const auto card = Cards::FindCardByID(id);
        REQUIRE_FALSE(card.id.empty());
        CHECK(card.gameTags.contains(GameTag::DIVINE_SHIELD));
        CHECK(card.gameTags.contains(GameTag::WINDFURY));
    }

    for (const auto* id : { "BGS_131", "TB_BaconUps_251" })
    {
        const auto card = Cards::FindCardByID(id);
        REQUIRE_FALSE(card.id.empty());
        CHECK(card.gameTags.contains(GameTag::VENOMOUS));
    }
}
