#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Battlegrounds : TrinketBehaviors] - complete immediate gold batch")
{
    const auto behavior = FindTrinketBehavior("BG30_MagicItem_996");
    CHECK(behavior.effect == TrinketEffect::GOLD_AND_MAX_GOLD);
    CHECK(behavior.value == 4);

    const auto bobblehead = FindTrinketBehavior("BG30_MagicItem_998");
    CHECK(bobblehead.effect == TrinketEffect::IMMEDIATE_GOLD);
    CHECK(bobblehead.value == 2);

    const auto gauntlet = FindTrinketBehavior("BG30_MagicItem_841");
    CHECK(gauntlet.effect == TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT);
    CHECK(gauntlet.attack == 3);
    CHECK(gauntlet.health == 3);
    CHECK(gauntlet.value == 1);

    const auto crystal = FindTrinketBehavior("BG36_MagicItem_220");
    CHECK(crystal.effect == TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE);

    const auto music = FindTrinketBehavior("BG30_MagicItem_430");
    CHECK(music.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS);
    CHECK(music.amount == 1);
    CHECK(music.repeatAtStartTurn);
    CHECK(music.battlecryOnly);
    CHECK(!music.magneticOnly);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - portrait acquisitions")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_301").cardID == "BG25_008");
    CHECK(FindTrinketBehavior("BG30_MagicItem_303").cardID == "BG_TTN_401");
    CHECK(FindTrinketBehavior("BG30_MagicItem_310").cardID == "BG25_354");
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - recurring fixed acquisition")
{
    const auto behavior = FindTrinketBehavior("BG30_MagicItem_406");
    CHECK(behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD);
    CHECK(behavior.cardID == "BG28_604");
    CHECK(behavior.repeatAtStartTurn);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - portrait fixed acquisitions")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_821").cardID == "LT23_809H");
    CHECK(FindTrinketBehavior("BG30_MagicItem_876").cardID == "BG_EX1_564");
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - recurring portrait acquisitions")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_831").cardID == "BGS_115");
    CHECK(FindTrinketBehavior("BG30_MagicItem_831").repeatAtStartTurn);
    CHECK(FindTrinketBehavior("BG30_MagicItem_944").cardID == "BG28_169");
    CHECK(FindTrinketBehavior("BG30_MagicItem_944").repeatAtStartTurn);
    CHECK(FindTrinketBehavior("BG30_MagicItem_987").cardID == "BG26_814");
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - tavern spell stat aura family")
{
    for (const auto* id : {"BG32_MagicItem_700", "BG32_MagicItem_801",
                           "BG32_MagicItem_801t", "BG36_MagicItem_373"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::TAVERN_SPELL_STATS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - trigger effects fail closed")
{
    for (const auto* id : {"BG30_MagicItem_541", "BG30_MagicItem_879",
                           "BG30_MagicItem_879t", "BG30_MagicItem_423",
                           "BG30_MagicItem_973",
                           "BG30_MagicItem_420"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - end of recruit family")
{
    const auto wallet = FindTrinketBehavior("BG30_MagicItem_847");
    CHECK(wallet.effect == TrinketEffect::END_TURN_MAX_GOLD);
    CHECK(wallet.value == 1);
    const auto anchor = FindTrinketBehavior("BG32_MagicItem_231");
    CHECK(anchor.effect == TrinketEffect::END_TURN_GOLDEN_STATS);
    CHECK(anchor.attack == 3);
    CHECK(anchor.health == 3);
    const auto golden = FindTrinketBehavior("BG32_MagicItem_231t");
    CHECK(golden.effect == TrinketEffect::END_TURN_GOLDEN_STATS);
    CHECK(golden.attack == 10);
    CHECK(golden.health == 10);
    const auto phrasebook = FindTrinketBehavior("BG30_MagicItem_914");
    CHECK(phrasebook.effect == TrinketEffect::AFTER_PLAY_HAND_BUFF);
    CHECK(phrasebook.attack == 3);
    CHECK(phrasebook.health == 3);
    const auto goldenPhrasebook = FindTrinketBehavior("BG30_MagicItem_914t");
    CHECK(goldenPhrasebook.attack == 6);
    CHECK(goldenPhrasebook.health == 6);
    const auto nomi = FindTrinketBehavior("BG30_MagicItem_544");
    CHECK(nomi.effect == TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF);
    CHECK(nomi.attack == 2);
    CHECK(nomi.health == 2);
    const auto goldenNomi = FindTrinketBehavior("BG30_MagicItem_544t");
    CHECK(goldenNomi.attack == 5);
    CHECK(goldenNomi.health == 5);
    const auto bloodletter = FindTrinketBehavior("BG36_MagicItem_800");
    CHECK(bloodletter.effect == TrinketEffect::AFTER_TAVERN_SPELL_SHOP_BUFF);
    const auto ship = FindTrinketBehavior("BG35_MagicItem_710");
    CHECK(ship.effect == TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF);
    CHECK(ship.race == Race::PIRATE);
    const auto pouch = FindTrinketBehavior("BG30_MagicItem_414");
    CHECK(pouch.effect == TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF);
    CHECK(pouch.amount == 2);
    const auto goldenPouch = FindTrinketBehavior("BG30_MagicItem_414t");
    CHECK(goldenPouch.attack == 4);
    CHECK(goldenPouch.health == 4);
    const auto shield = FindTrinketBehavior("BG30_MagicItem_984");
    CHECK(shield.effect == TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK);
    CHECK(shield.attack == 3);
    const auto goldenShield = FindTrinketBehavior("BG30_MagicItem_984t");
    CHECK(goldenShield.attack == 7);
    const auto dragon = FindTrinketBehavior("BG30_MagicItem_900");
    CHECK(dragon.effect == TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF);
    CHECK(dragon.race == Race::DRAGON);
    const auto goldenDragon = FindTrinketBehavior("BG30_MagicItem_900t");
    CHECK(goldenDragon.attack == 6);
    const auto undead = FindTrinketBehavior("BG30_MagicItem_989");
    CHECK(undead.effect == TrinketEffect::STATIC_RACE_STATS);
    CHECK(undead.race == Race::UNDEAD);
    CHECK(undead.attack == 3);
    const auto goldenUndead = FindTrinketBehavior("BG30_MagicItem_989t");
    CHECK(goldenUndead.attack == 15);
    const auto dragonsEye = FindTrinketBehavior("BG36_MagicItem_215");
    CHECK(dragonsEye.effect == TrinketEffect::DUPLICATE_DRAGON_BATTLECRY);
    const auto firstMinionShield = FindTrinketBehavior("BG36_MagicItem_811");
    CHECK(firstMinionShield.effect == TrinketEffect::FIRST_MINION_DIVINE_SHIELD);
    const auto warcryTotem = FindTrinketBehavior("BG36_MagicItem_202");
    CHECK(warcryTotem.effect == TrinketEffect::BATTLECRY_BUY_DISCOUNT);
    CHECK(warcryTotem.value == 2);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Kodo Leather Pouch purchase family")
{
    const auto normal = FindTrinketBehavior("BG30_MagicItem_414");
    CHECK(normal.effect == TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF);
    CHECK(normal.amount == 2);
    CHECK(normal.attack == 2);
    CHECK(normal.health == 1);
    const auto golden = FindTrinketBehavior("BG30_MagicItem_414t");
    CHECK(golden.effect == TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF);
    CHECK(golden.amount == 2);
    CHECK(golden.attack == 4);
    CHECK(golden.health == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - registrations")
{
    std::map<std::string, CardDef> cards;
    TrinketBehaviors::AddAll(cards);
    CHECK(cards.size() == 27);
    CHECK(cards.contains("BG30_MagicItem_996"));
    CHECK(cards.contains("BG30_MagicItem_841"));
    CHECK(cards.contains("BG36_MagicItem_220"));
    CHECK(cards.contains("BG32_MagicItem_858"));
    CHECK(cards.contains("BG30_MagicItem_993"));
    CHECK(cards.contains("BG30_MagicItem_430"));
    CHECK(cards.contains("BG30_MagicItem_998"));
    CHECK(cards.contains("BG30_MagicItem_847"));
    CHECK(cards.contains("BG32_MagicItem_231"));
    CHECK(cards.contains("BG32_MagicItem_231t"));
    CHECK(cards.contains("BG30_MagicItem_914"));
    CHECK(cards.contains("BG30_MagicItem_914t"));
    CHECK(cards.contains("BG30_MagicItem_544"));
    CHECK(cards.contains("BG30_MagicItem_544t"));
    CHECK(cards.contains("BG36_MagicItem_800"));
    CHECK(cards.contains("BG35_MagicItem_710"));
    CHECK(cards.contains("BG30_MagicItem_414"));
    CHECK(cards.contains("BG30_MagicItem_414t"));
    CHECK(cards.contains("BG30_MagicItem_984"));
    CHECK(cards.contains("BG30_MagicItem_984t"));
    CHECK(cards.contains("BG30_MagicItem_900"));
    CHECK(cards.contains("BG30_MagicItem_900t"));
    CHECK(cards.contains("BG30_MagicItem_989"));
    CHECK(cards.contains("BG30_MagicItem_989t"));
}
