// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <variant>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 8 inventory table")
{
    constexpr std::array ids{ "BGS_018",          "TB_BaconUps_085",
                              "BGS_041",          "TB_BaconUps_109",
                              "BGS_071",          "TB_BaconUps_123",
                              "BGS_116",          "TB_BaconUps_167",
                              "BGS_127",          "TB_Baconups_202" };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch8::AddAll(cards);
    GeneratedBehaviorMappings::AddAll(cards);

    for (const auto* id : ids)
    {
        CAPTURE(id);
        REQUIRE(cards.contains(id));
        const auto metadata = Cards::FindCardByID(id);
        REQUIRE_FALSE(metadata.id.empty());
        CHECK_EQ(metadata.id, id);
    }
    // HearthstoneJSON's pinned 36.4 snapshot omits the generated golden
    // enchantment records.  Batch8 intentionally reuses the corresponding
    // normal enchantment twice for golden effects, so only these five
    // metadata-backed enchantments are registered.
    constexpr std::array enchantmentIDs{
        "BGS_018e", "BGS_041e", "BGS_071e", "BGS_127e", "TB_Baconups_203e" };
    for (const auto* id : enchantmentIDs)
    {
        CAPTURE(id);
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetEnchant().has_value());
    }
    CHECK_EQ(cards.size(), ids.size() + enchantmentIDs.size());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 8 race buff scaling")
{
    struct Expected
    {
        const char* id;
        const char* enchantment;
        Race race;
    };
    constexpr std::array expected{
        Expected{ "BGS_018", "BGS_018e", Race::BEAST },
        Expected{ "TB_BaconUps_085", "BGS_018e", Race::BEAST },
        Expected{ "BGS_041", "BGS_041e", Race::DRAGON },
        Expected{ "TB_BaconUps_109", "BGS_041e", Race::DRAGON },
    };

    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch8::AddAll(cards);
    for (const auto& row : expected)
    {
        CAPTURE(row.id);
        REQUIRE(cards.contains(row.id));
        auto& def = cards.at(row.id);
        const auto expectedDeathrattleTasks =
            row.id == std::string_view{ "BGS_018" }
                ? 1
                : row.id == std::string_view{ "TB_BaconUps_085" } ? 2 : 0;
        REQUIRE_EQ(def.power.GetDeathrattleTask().size(),
                   expectedDeathrattleTasks);
        if (row.id == std::string_view{ "BGS_018" } ||
            row.id == std::string_view{ "TB_BaconUps_085" })
        {
            const auto* task = std::get_if<FriendlyRaceEnchantmentTask>(
                &def.power.GetDeathrattleTask().front());
            CHECK(task != nullptr);
        }
        else
        {
            REQUIRE(def.power.GetTrigger().has_value());
            CHECK_EQ(def.power.GetTrigger()->GetTriggerType(),
                     TriggerType::AFTER_PLAY_MINION);
            CHECK_EQ(def.power.GetTrigger()->GetTriggerSource(),
                     TriggerSource::MINIONS_EXCEPT_SELF);
        }
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 8 combat and refresh")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch8::AddAll(cards);

    for (const auto* id : { "BGS_071", "TB_BaconUps_123" })
    {
        CAPTURE(id);
        REQUIRE(cards.at(id).power.GetTrigger().has_value());
        CHECK_EQ(cards.at(id).power.GetTrigger()->GetTriggerType(),
                 TriggerType::SUMMON);
        CHECK_EQ(cards.at(id).power.GetTrigger()->GetTriggerSource(),
                 TriggerSource::MINIONS_EXCEPT_SELF);
    }

    REQUIRE_EQ(cards.at("BGS_116").power.GetBattlecryTask().size(), 1);
    REQUIRE_EQ(cards.at("TB_BaconUps_167").power.GetBattlecryTask().size(), 1);
    CHECK(std::holds_alternative<FreeRefreshTask>(
        cards.at("BGS_116").power.GetBattlecryTask().front()));
    CHECK(std::holds_alternative<FreeRefreshTask>(
        cards.at("TB_BaconUps_167").power.GetBattlecryTask().front()));

    for (const auto* id : { "BGS_127", "TB_Baconups_202" })
    {
        CAPTURE(id);
        REQUIRE(cards.at(id).power.GetTrigger().has_value());
        CHECK_EQ(cards.at(id).power.GetTrigger()->GetTriggerType(),
                 TriggerType::AFTER_PLAY_MINION);
    }
}
