#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : TrinketBehaviors] - complete immediate gold batch")
{
    const auto behavior = FindTrinketBehavior("BG30_MagicItem_996");
    CHECK(behavior.effect == TrinketEffect::GOLD_AND_MAX_GOLD);
    CHECK(behavior.value == 4);

    const auto gauntlet = FindTrinketBehavior("BG30_MagicItem_841");
    CHECK(gauntlet.effect == TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT);
    CHECK(gauntlet.attack == 3);
    CHECK(gauntlet.health == 3);
    CHECK(gauntlet.value == 1);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - trigger effects fail closed")
{
    for (const auto* id : {"BG30_MagicItem_541", "BG30_MagicItem_879",
                           "BG30_MagicItem_879t", "BG30_MagicItem_423",
                           "BG30_MagicItem_973", "BG30_MagicItem_847",
                           "BG30_MagicItem_420"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - registrations")
{
    std::map<std::string, CardDef> cards;
    TrinketBehaviors::AddAll(cards);
    CHECK(cards.size() == 2);
    CHECK(cards.contains("BG30_MagicItem_996"));
    CHECK(cards.contains("BG30_MagicItem_841"));
}
