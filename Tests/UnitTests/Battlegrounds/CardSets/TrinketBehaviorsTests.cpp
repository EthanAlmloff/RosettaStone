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
                           "BG30_MagicItem_420"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - refresh and self damage progress")
{
    const auto cheese = FindTrinketBehavior("BG30_MagicItem_879");
    CHECK(cheese.effect == TrinketEffect::REFRESH_SHOP_STATS);
    CHECK(cheese.attack == 1);
    CHECK(cheese.health == 1);
    CHECK(cheese.value == 4);
    CHECK(cheese.amount == 1);
    const auto goldenCheese = FindTrinketBehavior("BG30_MagicItem_879t");
    CHECK(goldenCheese.effect == TrinketEffect::REFRESH_SHOP_STATS);
    CHECK(goldenCheese.attack == 2);
    CHECK(goldenCheese.health == 2);
    const auto pendant = FindTrinketBehavior("BG30_MagicItem_541");
    CHECK(pendant.effect == TrinketEffect::HERO_DAMAGE_SHOP_STATS);
    CHECK(pendant.value == 3);
    CHECK(pendant.amount == 1);
    CHECK(FindTrinketBehavior("BG30_MagicItem_423").effect ==
          TrinketEffect::HIGHER_TIER_REFRESH);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - permanent minion and Blood Gem auras")
{
    const auto talisman = FindTrinketBehavior("BG30_MagicItem_880");
    CHECK(talisman.effect == TrinketEffect::STATIC_MINION_STATS);
    CHECK(talisman.attack == 2);
    CHECK(talisman.health == 1);
    const auto goldenTalisman = FindTrinketBehavior("BG30_MagicItem_880t");
    CHECK(goldenTalisman.attack == 8);
    CHECK(goldenTalisman.health == 5);
    const auto sticker = FindTrinketBehavior("BG30_MagicItem_988");
    CHECK(sticker.effect == TrinketEffect::BLOOD_GEM_BONUS);
    CHECK(sticker.attack == 2);
    CHECK(sticker.health == 1);
    CHECK(sticker.amount == 3);
    const auto goldenSticker = FindTrinketBehavior("BG30_MagicItem_988t");
    CHECK(goldenSticker.attack == 3);
    CHECK(goldenSticker.health == 3);
    CHECK(goldenSticker.amount == 5);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - combat-start and race trigger batch")
{
    const auto medal = FindTrinketBehavior("BG30_MagicItem_970");
    CHECK(medal.effect == TrinketEffect::START_COMBAT_MINION_STATS);
    CHECK(medal.attack == 2);
    CHECK(medal.health == 2);
    const auto goldenMedal = FindTrinketBehavior("BG30_MagicItem_970t");
    CHECK(goldenMedal.attack == 6);
    CHECK(goldenMedal.health == 6);
    const auto keychain = FindTrinketBehavior("BG30_MagicItem_843t");
    CHECK(keychain.effect == TrinketEffect::STATIC_TIER_MINION_STATS);
    CHECK(keychain.attack == 7);
    CHECK(keychain.health == 5);
    CHECK(keychain.value == 3);
    const auto boar = FindTrinketBehavior("BG30_MagicItem_547");
    CHECK(boar.effect == TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF);
    CHECK(boar.race == Race::UNDEAD);
    CHECK(boar.attack == 1);
    CHECK(FindTrinketBehavior("BG30_MagicItem_547t").attack == 2);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Reinforced Shield summon trigger")
{
    const auto shield = FindTrinketBehavior("BG30_MagicItem_886");
    CHECK(shield.effect == TrinketEffect::SUMMON_DIVINE_SHIELD);
    CHECK(shield.value == 5);
    CHECK(FindTrinketBehavior("BG30_MagicItem_886e").effect ==
          TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Naga and Mech event triggers")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_917").effect ==
          TrinketEffect::START_COMBAT_NAGA_SPELLCRAFT);
    CHECK(FindTrinketBehavior("BG30_MagicItem_919").effect ==
          TrinketEffect::AFTER_PLAY_NAGA_SPELLCRAFT);
    CHECK(FindTrinketBehavior("BG30_MagicItem_978").effect ==
          TrinketEffect::SUMMON_MECH_RANDOM_DIVINE_SHIELD);
    CHECK(FindTrinketBehavior("BG30_MagicItem_917e").effect ==
          TrinketEffect::NONE);
    CHECK(FindTrinketBehavior("BG30_MagicItem_978e").effect ==
          TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - summon and deathrattle event batch")
{
    const auto hoggy = FindTrinketBehavior("BG30_MagicItem_411");
    CHECK(hoggy.effect == TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GEMS);
    CHECK(hoggy.value == 2);
    CHECK(FindTrinketBehavior("BG30_MagicItem_540").effect ==
          TrinketEffect::SUMMON_BEAST_DOUBLE_ATTACK);
    const auto slamma = FindTrinketBehavior("BG35_MagicItem_871");
    CHECK(slamma.effect == TrinketEffect::SUMMON_BEAST_STATS);
    CHECK(slamma.attack == 6);
    CHECK(slamma.health == 6);
    CHECK(FindTrinketBehavior("BG30_MagicItem_411e").effect ==
          TrinketEffect::NONE);
    CHECK(FindTrinketBehavior("BG30_MagicItem_540e").effect ==
          TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - beast summon cadence and elemental deathrattle")
{
    // Deathly Phylactery needs a Discover and first-deathrattle state
    // machine; a stale Beast-summon approximation is intentionally closed.
    CHECK(FindTrinketBehavior("BG30_MagicItem_700").effect ==
          TrinketEffect::NONE);
    CHECK(FindTrinketBehavior("BG30_MagicItem_952").effect ==
          TrinketEffect::START_COMBAT_ELEMENTAL_FROSTLING);
    CHECK(FindTrinketBehavior("BG30_MagicItem_952e").effect ==
          TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Fang Anklet summon scaling")
{
    const auto fang = FindTrinketBehavior("BG35_MagicItem_701");
    CHECK(fang.effect == TrinketEffect::START_COMBAT_BEAST_SCALING);
    CHECK(fang.attack == 1);
    CHECK(fang.health == 1);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Blood Golem Sticker")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_442").effect ==
          TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GOLEM);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - edge shield and combat copy")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_902").effect ==
          TrinketEffect::START_COMBAT_EDGE_SHIELDS);
    CHECK(FindTrinketBehavior("BG30_MagicItem_972").effect ==
          TrinketEffect::START_COMBAT_LEFT_COPY);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Undead reborn and deathrattle start")
{
    CHECK(FindTrinketBehavior("BG32_MagicItem_360").effect ==
          TrinketEffect::START_COMBAT_UNDEAD_EDGE_REBORN);
    CHECK(FindTrinketBehavior("BG32_MagicItem_306").effect ==
          TrinketEffect::START_COMBAT_TRIGGER_DEATHRATTLES);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Crocheted Sungill")
{
    const auto sungill = FindTrinketBehavior("BG32_MagicItem_960");
    CHECK(sungill.effect == TrinketEffect::START_COMBAT_HIGHEST_HAND_MINION);
    CHECK(sungill.attack == 4);
    CHECK(sungill.health == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Unholy Sanctum")
{
    const auto sanctum = FindTrinketBehavior("BG32_MagicItem_862");
    CHECK(sanctum.effect == TrinketEffect::AFTER_DEATHRATTLE_RIGHTMOST_STATS);
    CHECK(sanctum.attack == 2);
    CHECK(sanctum.health == 2);
    const auto golden = FindTrinketBehavior("BG32_MagicItem_862t");
    CHECK(golden.attack == 6);
    CHECK(golden.health == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - neutral triple and dragon cap")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_403").effect ==
          TrinketEffect::START_COMBAT_NEUTRAL_TRIPLE);
    CHECK(FindTrinketBehavior("BG30_MagicItem_542").effect ==
          TrinketEffect::START_COMBAT_DRAGON_MAX_ATTACK);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - hand stat transfer and training")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_441").effect ==
          TrinketEffect::START_COMBAT_LEFTMOST_HAND_STATS);
    CHECK(FindTrinketBehavior("BG30_MagicItem_962").effect ==
          TrinketEffect::START_COMBAT_LOWEST_ATTACK_DOUBLE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Stegodon Portrait")
{
    CHECK(FindTrinketBehavior("BG35_MagicItem_702").effect ==
          TrinketEffect::START_COMBAT_LEFT_BEAST_SHIELDS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Yu'lon Sticker")
{
    CHECK(FindTrinketBehavior("BG32_MagicItem_419").effect ==
          TrinketEffect::START_COMBAT_HIGHEST_TIER_DRAGON_GOLDEN);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Hogwash Basin")
{
    CHECK(FindTrinketBehavior("BG32_MagicItem_904").effect ==
          TrinketEffect::START_COMBAT_THREE_BLOOD_GEMS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Eclectic Shrine and Anemone")
{
    CHECK(FindTrinketBehavior("BG32_MagicItem_280").effect == TrinketEffect::START_COMBAT_TYPE_STATS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Protective Ring")
{
    const auto ring = FindTrinketBehavior("BG35_MagicItem_711");
    CHECK(ring.effect == TrinketEffect::START_COMBAT_RANDOM_PIRATE_SHIELDS);
    CHECK(ring.value == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Dramaloc Sticker")
{
    CHECK(FindTrinketBehavior("BG35_MagicItem_754").effect ==
          TrinketEffect::START_COMBAT_MURLOC_MAX_ATTACK);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Lightfeather Sticker")
{
    CHECK(FindTrinketBehavior("BG36_MagicItem_213").effect ==
          TrinketEffect::START_COMBAT_RALLY_SHIELDS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Quilligraphy Set")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_410").effect == TrinketEffect::AVENGE_BLOOD_GEM_BONUS);
    CHECK(FindTrinketBehavior("BG30_MagicItem_410").value == 3);
    CHECK(FindTrinketBehavior("BG30_MagicItem_410t2").value == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Staff of the Scourge")
{
    const auto staff = FindTrinketBehavior("BG30_MagicItem_437");
    CHECK(staff.effect == TrinketEffect::AVENGE_RANDOM_UNDEAD_REBORN);
    CHECK(staff.value == 5);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Alliance Keychain")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_433").effect ==
          TrinketEffect::FIRST_DEATH_MAX_STATS_RANDOM);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Fridge Magnet")
{
    const auto fridge = FindTrinketBehavior("BG30_MagicItem_545");
    CHECK(fridge.effect == TrinketEffect::AVENGE_RANDOM_MAGNETIC);
    CHECK(fridge.value == 3);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Jar o' Gems")
{
    const auto jar = FindTrinketBehavior("BG30_MagicItem_546");
    CHECK(jar.effect == TrinketEffect::AFTER_TWO_ATTACKS_QUILBOAR_GEM);
    CHECK(jar.value == 2);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Vash'jir Anemone spell scaling")
{
    const auto anemone = FindTrinketBehavior("BG32_MagicItem_932");
    CHECK(anemone.effect == TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - fodder and sold-minion Tavern auras")
{
    const auto fodder = FindTrinketBehavior("BG35_MagicItem_151");
    CHECK(fodder.effect == TrinketEffect::STATIC_FODDER_SHOP_STATS);
    CHECK(fodder.attack == 4);
    CHECK(fodder.health == 4);
    const auto goldenFodder = FindTrinketBehavior("BG35_MagicItem_151t");
    CHECK(goldenFodder.attack == 15);
    CHECK(goldenFodder.health == 15);
    const auto pie = FindTrinketBehavior("BG30_MagicItem_992");
    CHECK(pie.effect == TrinketEffect::TAVERN_STATS_PER_SOLD);
    CHECK(pie.attack == 1);
    CHECK(pie.health == 1);
    const auto goldenPie = FindTrinketBehavior("BG30_MagicItem_992t");
    CHECK(goldenPie.attack == 2);
    CHECK(goldenPie.health == 2);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Tavern spell economy and turn economy")
{
    const auto terrarium = FindTrinketBehavior("BG30_MagicItem_979");
    CHECK(terrarium.effect == TrinketEffect::NEXT_TAVERN_SPELL_DISCOUNT);
    CHECK(terrarium.value == 1);
    const auto necklace = FindTrinketBehavior("BG35_MagicItem_921");
    CHECK(necklace.effect == TrinketEffect::STAT_TAVERN_SPELL_DISCOUNT);
    CHECK(necklace.value == 2);
    const auto candle = FindTrinketBehavior("BG30_MagicItem_986");
    CHECK(candle.effect == TrinketEffect::FREE_TAVERN_SPELL_USES);
    CHECK(candle.value == 3);
    const auto wax = FindTrinketBehavior("BG32_MagicItem_823");
    CHECK(wax.effect == TrinketEffect::START_TURN_GOLD_DAMAGE);
    CHECK(wax.attack == 2);
    CHECK(wax.value == 2);
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

TEST_CASE("[Battlegrounds : TrinketBehaviors] - refresh, avenge, and damage aura batch")
{
    const auto crystal = FindTrinketBehavior("BG35_MagicItem_150");
    CHECK(crystal.effect == TrinketEffect::REFRESH_TEMP_SHOP_STATS);
    CHECK(crystal.attack == 3);
    CHECK(crystal.health == 3);
    const auto avenge = FindTrinketBehavior("BG30_MagicItem_864");
    CHECK(avenge.effect == TrinketEffect::AVENGE_MINION_STATS);
    CHECK(avenge.value == 3);
    CHECK(avenge.attack == 4);
    CHECK(avenge.health == 5);
    CHECK(avenge.amount == 1);
    const auto damage = FindTrinketBehavior("BG35_MagicItem_155");
    CHECK(damage.effect == TrinketEffect::TAVERN_SPELL_TEMP_STATS_AFTER_DAMAGE);
    CHECK(damage.attack == 1);
    CHECK(damage.health == 1);
    const auto pirates = FindTrinketBehavior("BG30_MagicItem_923");
    CHECK(pirates.effect == TrinketEffect::PIRATE_ATTACK_GOLD);
    CHECK(pirates.race == Race::PIRATE);
    CHECK(pirates.value == 2);
    CHECK(pirates.amount == 1);
    const auto sword = FindTrinketBehavior("BG30_MagicItem_925");
    CHECK(sword.effect == TrinketEffect::ATTACKING_MINION_STATS);
    CHECK(sword.attack == 4);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - registrations")
{
    std::map<std::string, CardDef> cards;
    TrinketBehaviors::AddAll(cards);
    CHECK(cards.size() == 93);
    CHECK(cards.contains("BG30_MagicItem_996"));
    CHECK(cards.contains("BG30_MagicItem_841"));
    CHECK(cards.contains("BG36_MagicItem_220"));
    CHECK(cards.contains("BG30_MagicItem_541"));
    CHECK(cards.contains("BG30_MagicItem_879"));
    CHECK(cards.contains("BG30_MagicItem_879t"));
    CHECK(cards.contains("BG30_MagicItem_423"));
    CHECK(cards.contains("BG30_MagicItem_880"));
    CHECK(cards.contains("BG30_MagicItem_880t"));
    CHECK(cards.contains("BG30_MagicItem_988"));
    CHECK(cards.contains("BG30_MagicItem_988t"));
    CHECK(cards.contains("BG30_MagicItem_970"));
    CHECK(cards.contains("BG30_MagicItem_970t"));
    CHECK(cards.contains("BG30_MagicItem_843t"));
    CHECK(cards.contains("BG30_MagicItem_547"));
    CHECK(cards.contains("BG30_MagicItem_547t"));
    CHECK(cards.contains("BG35_MagicItem_150"));
    CHECK(cards.contains("BG30_MagicItem_864"));
    CHECK(cards.contains("BG35_MagicItem_155"));
    CHECK(cards.contains("BG30_MagicItem_923"));
    CHECK(cards.contains("BG30_MagicItem_925"));
    CHECK(cards.contains("BG30_MagicItem_995"));
    CHECK(cards.contains("BG32_MagicItem_270"));
    CHECK(cards.contains("BG30_MagicItem_973"));
    CHECK(cards.contains("BG30_MagicItem_934"));
    CHECK(cards.contains("BG32_MagicItem_276"));
    CHECK(cards.contains("BG35_MagicItem_814"));
    CHECK(cards.contains("BG32_MagicItem_428"));
    CHECK(cards.contains("BG35_MagicItem_923"));
    CHECK(cards.contains("BG35_MagicItem_151"));
    CHECK(cards.contains("BG35_MagicItem_151t"));
    CHECK(cards.contains("BG30_MagicItem_992"));
    CHECK(cards.contains("BG30_MagicItem_992t"));
    CHECK(cards.contains("BG30_MagicItem_979"));
    CHECK(cards.contains("BG35_MagicItem_921"));
    CHECK(cards.contains("BG30_MagicItem_986"));
    CHECK(cards.contains("BG32_MagicItem_823"));
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
    CHECK(cards.contains("BG32_MagicItem_960"));
    CHECK(cards.contains("BG32_MagicItem_862"));
    CHECK(cards.contains("BG32_MagicItem_862t"));
    CHECK(cards.contains("BG30_MagicItem_403"));
    CHECK(cards.contains("BG30_MagicItem_542"));
    CHECK(cards.contains("BG30_MagicItem_441"));
    CHECK(cards.contains("BG30_MagicItem_962"));
    CHECK(cards.contains("BG35_MagicItem_702"));
    CHECK(cards.contains("BG32_MagicItem_419"));
    CHECK(cards.contains("BG32_MagicItem_904"));
    CHECK(cards.contains("BG32_MagicItem_280"));
    CHECK(cards.contains("BG35_MagicItem_711"));
    CHECK(cards.contains("BG35_MagicItem_754"));
    CHECK(cards.contains("BG36_MagicItem_213"));
}
