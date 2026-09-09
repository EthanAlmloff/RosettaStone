#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch25.hpp>
#include <Rosetta/Battlegrounds/Actions/Generic.hpp>
#include <Rosetta/Battlegrounds/Cards/Card.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
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

TEST_CASE("Batch25 attaches the exact normal and golden enchantment children")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch25::AddAll(cards);

    const auto checkTargeted = [&cards](const char* parent, const char* child) {
        const auto& tasks = cards.at(parent).power.GetBattlecryTask();
        REQUIRE(tasks.size() >= 1);
        REQUIRE(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(
            tasks.front()));
        const auto& task = std::get<SimpleTasks::AddEnchantmentTask>(tasks.front());
        CHECK(task.CardID() == child);
        CHECK(task.Entity() == EntityType::TARGET);
    };

    checkTargeted("BG25_004", "BG25_004e");
    checkTargeted("BG30_756", "BG30_756e");

    const auto checkRaceDeath = [&cards](const char* parent, const char* child) {
        const auto& tasks = cards.at(parent).power.GetBattlecryTask();
        REQUIRE(tasks.size() == 1);
        REQUIRE(std::holds_alternative<SimpleTasks::FriendlyRaceEnchantmentTask>(
            tasks.front()));
        const auto& task = std::get<SimpleTasks::FriendlyRaceEnchantmentTask>(
            tasks.front());
        CHECK(task.CardID() == child);
    };

    checkRaceDeath("BG32_824", "BG32_824e");
    checkRaceDeath("BG26_522", "BG26_522e");
}

TEST_CASE("Batch25 does not silently credit an unrelated enchantment child")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch25::AddAll(cards);
    const auto& tasks = cards.at("BG25_004").power.GetBattlecryTask();
    REQUIRE(tasks.size() == 1);
    REQUIRE(std::holds_alternative<SimpleTasks::AddEnchantmentTask>(tasks.front()));
    const auto& task = std::get<SimpleTasks::AddEnchantmentTask>(tasks.front());
    CHECK(task.CardID() != "BG25_004e2");
    CHECK(task.CardID() != "BG30_756e");
}

TEST_CASE("Batch25 child CardDefs have executable payloads for Generic::AddEnchantment")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch25::AddAll(cards);

    // The source registration is not sufficient: Generic::AddEnchantment
    // only executes the child Card's optional Enchant payload.  Exercise that
    // same path against every normal/golden child registered by this batch.
    const auto checkEnchant = [&cards](const char* child, int attack,
                                       int health) {
        REQUIRE(cards.contains(child));
        const auto& definition = cards.at(child);
        REQUIRE(definition.power.GetEnchant().has_value());

        Card source;
        source.id = "BATCH25_TEST_TARGET";
        source.gameTags[GameTag::ATK] = 10;
        source.gameTags[GameTag::HEALTH] = 10;
        Minion target(source);

        Card enchantment;
        enchantment.id = child;
        enchantment.power = definition.power;
        Generic::AddEnchantment(enchantment, target);
        CHECK(target.GetAttack() == 10 + attack);
        CHECK(target.GetHealth() == 10 + health);
    };

    checkEnchant("BG25_004e", 2, 7);
    checkEnchant("BG25_004Ge", 4, 14);
    checkEnchant("BG26_522e", 2, 2);
    checkEnchant("BG26_522Ge", 2, 2);
    checkEnchant("BG30_756e", 2, 3);
    checkEnchant("BG30_756Ge", 4, 6);
    checkEnchant("BG32_824e", 10, 0);
    checkEnchant("BG32_824Ge", 20, 0);
    checkEnchant("BG33_701e", 2, 2);
    checkEnchant("BG33_701Ge", 4, 4);
}
