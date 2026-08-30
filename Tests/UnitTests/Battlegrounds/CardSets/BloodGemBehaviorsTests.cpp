#include <Rosetta/Battlegrounds/CardSets/BloodGemBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>

#include <doctest/doctest.h>

#include <utility>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("Blood Gem uses canonical targeted spell behavior")
{
    const auto behavior = FindTavernSpellBehavior("BG20_GEM");
    CHECK(behavior.gold == 0);
    CHECK(behavior.attack == 1);
    CHECK(behavior.health == 1);
    CHECK(behavior.effect == TavernSpellEffect::BLOOD_GEM);
    CHECK(TavernSpellRequiresTarget(behavior.effect));
}

TEST_CASE("Blood Gem behavior registry covers exact modeled Quilboar set")
{
    std::map<std::string, CardDef> cards;
    BloodGemBehaviors::AddAll(cards);
    CHECK(cards.size() == 19);
    for (const auto* id : { "BG20_GEM", "BG20_100", "BG20_100_G",
                            "BG20_203", "BG20_203_G",
                            "BG20_205", "BG20_205_G", "BG20_207",
                            "BG20_207_G", "BG20_301", "BG20_301_G",
                            "BG20_103",
                            "BG20_103_G", "BG20_102", "BG20_102_G",
                            "BG20_105", "BG20_105_G" })
    {
        CHECK(cards.contains(id));
    }

    for (const auto* id : { "BG20_302", "BG20_302_G" })
    {
        CHECK_FALSE(cards.contains(id));
    }
}

TEST_CASE("Thorncaller generates exact normal and golden Blood Gem counts")
{
    const auto gem = Cards::FindCardByID("BG20_GEM");
    REQUIRE(gem.id == "BG20_GEM");
    CHECK(gem.GetCardType() == CardType::BATTLEGROUND_SPELL);
    std::map<std::string, CardDef> cards;
    BloodGemBehaviors::AddAll(cards);
    for (const auto [id, amount] : { std::pair{"BG20_105", 1},
                                    std::pair{"BG20_105_G", 2} })
    {
        REQUIRE(cards.contains(id));
        auto& power = cards.at(id).power;
        REQUIRE(power.GetBattlecryTask().size() == 1);
        REQUIRE(power.GetDeathrattleTask().size() == 1);
        CHECK(std::holds_alternative<SimpleTasks::GenerateBloodGemsTask>(
            power.GetBattlecryTask().front()));
        CHECK(std::holds_alternative<SimpleTasks::GenerateBloodGemsTask>(
            power.GetDeathrattleTask().front()));
        CHECK(std::get<SimpleTasks::GenerateBloodGemsTask>(
                  power.GetBattlecryTask().front())
                  .Amount() == amount);
        CHECK(std::get<SimpleTasks::GenerateBloodGemsTask>(
                  power.GetDeathrattleTask().front())
                  .Amount() == amount);
    }
}
