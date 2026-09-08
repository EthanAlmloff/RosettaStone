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

TEST_CASE("[Battlegrounds : TrinketBehaviors] - fixed supported portrait batch")
{
    const std::pair<const char*, const char*> expected[] = {
        {"BG30_MagicItem_431", "BG28_707"},
        {"BG30_MagicItem_432", "BG26_888"},
        {"BG30_MagicItem_555", "BG26_175"},
        {"BG30_MagicItem_803", "BG_BOT_606"},
        {"BG30_MagicItem_825", "BG21_013"},
        {"BG30_MagicItem_828", "BG26_505"},
        {"BG30_MagicItem_869", "BG29_873"},
        {"BG30_MagicItem_971", "BGS_009"},
        {"BG32_MagicItem_274", "BG26_157"},
        {"BG32_MagicItem_301", "BG26_350"},
        {"BG32_MagicItem_804", "BG_OG_221"},
        {"BG35_MagicItem_741", "BG26_149"},
        {"BG35_MagicItem_742", "BG26_147"},
        {"BG35_MagicItem_924", "BG31_035"},
    };
    for (const auto& [id, card] : expected) {
        const auto behavior = FindTrinketBehavior(id);
        CHECK(behavior.effect == TrinketEffect::ACQUIRE_FIXED_CARD);
        CHECK(behavior.cardID == card);
        CHECK(behavior.amount == 1);
    }
    const auto sellemental = FindTrinketBehavior("BG32_MagicItem_831");
    CHECK(sellemental.effect == TrinketEffect::ACQUIRE_FIXED_CARD);
    CHECK(sellemental.cardID == "BGS_115");
    CHECK(sellemental.repeatAtStartTurn);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - portrait extras are explicit")
{
    const auto unsupported[] = {
        std::pair{"BG30_MagicItem_431", PortraitEffect::LIVING_AZERITE_ELEMENTAL_STATS},
        std::pair{"BG30_MagicItem_432", PortraitEffect::BELCHER_VENOMOUS_LOSS_STATS},
        std::pair{"BG30_MagicItem_555", PortraitEffect::SURPRISE_MORE_ELEMENTALS},
        std::pair{"BG30_MagicItem_803", PortraitEffect::KABOOM_BOT_DEATHRATTLE_DAMAGE},
        std::pair{"BG30_MagicItem_825", PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON},
        std::pair{"BG30_MagicItem_828", PortraitEffect::ZESTY_SHAKER_EXTRA_COPY},
        std::pair{"BG30_MagicItem_869", PortraitEffect::FELBLOOD_BOTH_STATS},
        std::pair{"BG30_MagicItem_971", PortraitEffect::LIGHTFANG_ALL_TYPES},
        std::pair{"BG32_MagicItem_274", PortraitEffect::BRISTLEBACH_ALL_MINIONS},
        std::pair{"BG32_MagicItem_301", PortraitEffect::BASSGILL_SUMMON_DIVINE_SHIELD},
        std::pair{"BG32_MagicItem_804", PortraitEffect::SELFLESS_BATTLECRY},
        std::pair{"BG35_MagicItem_924", PortraitEffect::GROUNDBREAKER_ADJACENT_STATS},
    };
    for (const auto& [id, effect] : unsupported)
    {
        const auto behavior = FindTrinketBehavior(id);
        CHECK(behavior.portraitEffect == effect);
        CHECK_FALSE(behavior.portraitExecutable);
    }
    for (const auto* id : {"BG32_MagicItem_831", "BG35_MagicItem_741",
                           "BG35_MagicItem_742"})
    {
        const auto behavior = FindTrinketBehavior(id);
        CHECK(behavior.portraitEffect == PortraitEffect::NONE);
        CHECK(behavior.portraitExecutable);
    }
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - tavern spell stat aura family")
{
    for (const auto* id : {"BG32_MagicItem_700", "BG32_MagicItem_801",
                           "BG32_MagicItem_801t", "BG36_MagicItem_373"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::TAVERN_SPELL_STATS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Season 14 counter and event batch")
{
    const auto pipe = FindTrinketBehavior("BG32_MagicItem_281");
    CHECK(pipe.effect == TrinketEffect::TAVERN_SPELL_NO_TYPE_STATS);
    CHECK(pipe.attack == 4);
    CHECK(pipe.health == 4);

    CHECK(FindTrinketBehavior("BG32_MagicItem_279").effect ==
          TrinketEffect::BLOOD_GEM_DIVINE_SHIELD);

    const auto scale = FindTrinketBehavior("BG32_MagicItem_363");
    CHECK(scale.effect == TrinketEffect::ATTACKING_DRAGON_DIVINE_SHIELD);
    CHECK(scale.value == 3);

    const auto kibble = FindTrinketBehavior("BG32_MagicItem_200");
    CHECK(kibble.effect == TrinketEffect::ATTACKING_BEAST_SCALING);
    CHECK(kibble.attack == 2);

    const auto aggem = FindTrinketBehavior("BG32_MagicItem_284");
    CHECK(aggem.effect == TrinketEffect::END_TURN_BLOOD_GEMS_PER_TYPE);
    CHECK(aggem.attack == 7);

    const auto stinger = FindTrinketBehavior("BG32_MagicItem_111");
    CHECK(stinger.effect == TrinketEffect::END_TURN_MURLOC_STATS);
    CHECK(stinger.attack == 8);
    CHECK(stinger.health == 8);

    const auto wrench = FindTrinketBehavior("BG32_MagicItem_170");
    CHECK(wrench.effect == TrinketEffect::AFTER_MAGNETIC_MECH_REPAIR);
    CHECK(wrench.attack == 4);
    CHECK(wrench.health == 8);
    CHECK(FindTrinketBehavior("BG32_MagicItem_364").cardID ==
          "BG34_Giant_314");
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Vashjir Anemone spell scaling")
{
    const auto anemone = FindTrinketBehavior("BG32_MagicItem_932");
    CHECK(anemone.effect == TrinketEffect::START_COMBAT_NAGA_HEALTH);
    CHECK(anemone.health == 1);
    CHECK(anemone.value == 4);
    CHECK(anemone.race == Race::NAGA);
}


TEST_CASE("[Battlegrounds : TrinketBehaviors] - trigger effects fail closed")
{
    for (const auto* id : {"BG30_MagicItem_541", "BG30_MagicItem_879",
                           "BG30_MagicItem_879t", "BG30_MagicItem_423"})
        CHECK(FindTrinketBehavior(id).effect == TrinketEffect::NONE);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Cathedral, Supply, Beads and follow-up trinkets")
{
    CHECK(FindTrinketBehavior("BG30_MagicItem_434").effect ==
          TrinketEffect::FIRST_SPELL_REPEAT);
    const auto supply = FindTrinketBehavior("BG30_MagicItem_435");
    CHECK(supply.effect == TrinketEffect::END_TURN_FIXED_CARD);
    CHECK(supply.value == 3);
    CHECK(supply.amount == 1);
    CHECK(supply.cardID == "BG26_813t");
    const auto beads = FindTrinketBehavior("BG30_MagicItem_982");
    CHECK(beads.effect == TrinketEffect::AFTER_BUY_BATTLECRY_MINION);
    CHECK(beads.value == 2);
    CHECK(beads.amount == 1);
    CHECK(beads.battlecryOnly);

    CHECK(FindTrinketBehavior("BG30_MagicItem_821t2").effect ==
          TrinketEffect::START_COMBAT_GOLDEN_FISH);
    const auto surveyor = FindTrinketBehavior("BG30_MagicItem_943");
    CHECK(surveyor.effect == TrinketEffect::ACQUIRE_FIXED_CARD);
    CHECK(surveyor.cardID == "BG30_121");
    const auto brew = FindTrinketBehavior("BG30_MagicItem_924");
    CHECK(brew.effect == TrinketEffect::SPEND_GOLD_PIRATE_STATS);
    CHECK(brew.attack == 3);
    CHECK(brew.health == 3);
    const auto goldenBrew = FindTrinketBehavior("BG30_MagicItem_924t");
    CHECK(goldenBrew.attack == 6);
    CHECK(goldenBrew.health == 6);

    const auto sushi = FindTrinketBehavior("BG30_MagicItem_920");
    CHECK(sushi.effect == TrinketEffect::SPELLCRAFT_REPEAT);
    CHECK(sushi.value == 2);
    const auto eye = FindTrinketBehavior("BG30_MagicItem_981");
    CHECK(eye.effect == TrinketEffect::AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL);
    const auto mushroom = FindTrinketBehavior("BG32_MagicItem_700");
    CHECK(mushroom.effect == TrinketEffect::TAVERN_SPELL_GROWING_STATS);
    CHECK(mushroom.attack == 1);
    CHECK(mushroom.health == 1);
    const auto honeycomb = FindTrinketBehavior("BG36_MagicItem_371");
    CHECK(honeycomb.effect ==
          TrinketEffect::TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Book of Medivh cadence")
{
    const auto book = FindTrinketBehavior("BG30_MagicItem_420");
    CHECK(book.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER);
    CHECK(book.value == 1);
    const auto golden = FindTrinketBehavior("BG30_MagicItem_420t");
    CHECK(golden.effect == TrinketEffect::BOOK_OF_MEDIVH_DISCOVER);
    CHECK(golden.value == 2);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Globe and Dishware lifecycle")
{
    const auto globe = FindTrinketBehavior("BG30_MagicItem_425");
    CHECK(globe.effect == TrinketEffect::AZEROTH_MODEL_GLOBE);
    CHECK(globe.value == 2);
    CHECK(globe.tier == 6);
    CHECK(globe.amount == 2);
    CHECK(FindTrinketBehavior("BG30_MagicItem_419").effect ==
          TrinketEffect::END_TURN_RANDOM_TYPE_MINIONS);
    CHECK(FindTrinketBehavior("BG30_MagicItem_425").amount == 2);
    CHECK(FindTrinketBehavior("BG30_MagicItem_419").tier == 0);
    const auto compass = FindTrinketBehavior("BG30_MagicItem_426");
    CHECK(compass.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS);
    CHECK(compass.repeatAtStartTurn);
    CHECK(compass.amount == 1);
    const auto goldenCompass = FindTrinketBehavior("BG30_MagicItem_426t");
    CHECK(goldenCompass.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS);
    CHECK(goldenCompass.repeatAtStartTurn);
    CHECK(goldenCompass.amount == 2);
    const auto pendant = FindTrinketBehavior("BG30_MagicItem_706");
    CHECK(pendant.effect == TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY);
    CHECK(pendant.repeatAtStartTurn);
    const auto essence = FindTrinketBehavior("BG30_MagicItem_916");
    CHECK(essence.effect == TrinketEffect::ACQUIRE_FIXED_CARD);
    CHECK(essence.cardID == "BG28_883");
    CHECK(essence.amount == 2);
    CHECK(essence.startTurnAmount == 1);
    CHECK(essence.repeatAtStartTurn);
    const auto sticker = FindTrinketBehavior("BG30_MagicItem_942");
    CHECK(sticker.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS);
    CHECK(sticker.race == Race::DEMON);
    CHECK(sticker.amount == 2);
    CHECK(sticker.magneticOnly);
    const auto claw = FindTrinketBehavior("BG30_MagicItem_930");
    CHECK(claw.effect == TrinketEffect::ACQUIRE_LAST_OPPONENT_COPY);
    CHECK(claw.repeatAtStartTurn);
    const auto glowscale = FindTrinketBehavior("BG30_MagicItem_548");
    CHECK(glowscale.effect == TrinketEffect::ACQUIRE_FIXED_GLOWSCALE);
    CHECK(glowscale.cardID == "BG34_Giant_035");
    CHECK(glowscale.amount == 1);
    const auto lionfish = FindTrinketBehavior("BG36_MagicItem_201");
    CHECK(lionfish.effect == TrinketEffect::ACQUIRE_FIXED_LIONFISH);
    CHECK(lionfish.cardID == "BG36_201");
    CHECK(lionfish.amount == 1);
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

TEST_CASE("[Battlegrounds : TrinketBehaviors] - combat and end-turn trinket batch")
{
    const auto scale = FindTrinketBehavior("BG32_MagicItem_363");
    CHECK(scale.effect == TrinketEffect::ATTACKING_DRAGON_DIVINE_SHIELD);
    CHECK(scale.value == 3);
    CHECK(FindTrinketBehavior("BG36_MagicItem_361").effect ==
          TrinketEffect::START_COMBAT_NAGA_DOUBLE_STATS);
    const auto mallet = FindTrinketBehavior("BG36_MagicItem_302");
    CHECK(mallet.effect == TrinketEffect::END_TURN_MINION_STATS);
    CHECK(mallet.attack == 2);
    CHECK(mallet.health == 1);
    const auto goldenMallet = FindTrinketBehavior("BG36_MagicItem_302t");
    CHECK(goldenMallet.attack == 4);
    CHECK(goldenMallet.health == 2);
    const auto ring = FindTrinketBehavior("BG36_MagicItem_371");
    CHECK(ring.effect == TrinketEffect::TAVERN_SPELL_STATS);
    CHECK(ring.attack == 1);
    CHECK(ring.health == 1);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Season 14 generic trigger batch")
{
    const auto rally = FindTrinketBehavior("BG36_MagicItem_200");
    CHECK(rally.effect == TrinketEffect::RALLY_ATTACK_FREE_REFRESH);
    const auto skull = FindTrinketBehavior("BG36_MagicItem_203");
    CHECK(skull.effect == TrinketEffect::BATTLECRY_EDGE_STATS);
    CHECK(skull.attack == 5);
    CHECK(skull.health == 5);
    CHECK(FindTrinketBehavior("BG36_MagicItem_212").effect ==
          TrinketEffect::END_TURN_LEFT_DEATHRATTLES);
    CHECK(FindTrinketBehavior("BG36_MagicItem_214").effect ==
          TrinketEffect::END_TURN_RALLY_TRIGGERS);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - Scraper Sticker random magnetic mech cadence")
{
    const auto scraper = FindTrinketBehavior("BG35_MagicItem_301");
    CHECK(scraper.effect == TrinketEffect::ACQUIRE_RANDOM_MINIONS);
    CHECK(scraper.race == Race::MECHANICAL);
    CHECK(scraper.amount == 1);
    CHECK(scraper.repeatAtStartTurn);
    CHECK(scraper.magneticOnly);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - sell and death random-minion thresholds")
{
    const auto check = [](std::string_view id, TrinketEffect effect,
                          Race race, int threshold) {
        const auto behavior = FindTrinketBehavior(id);
        CHECK(behavior.effect == effect);
        CHECK(behavior.race == race);
        CHECK(behavior.value == threshold);
    };
    check("BG30_MagicItem_710", TrinketEffect::AFTER_SELL_RANDOM_MINION,
          Race::MURLOC, 5);
    check("BG30_MagicItem_951", TrinketEffect::AFTER_SELL_RANDOM_MINION,
          Race::ELEMENTAL, 6);
    check("BG30_MagicItem_713", TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION,
          Race::UNDEAD, 8);
    check("BG30_MagicItem_931", TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION,
          Race::BEAST, 7);
    check("BG35_MagicItem_302", TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION,
          Race::MECHANICAL, 8);
}

TEST_CASE("[Battlegrounds : TrinketBehaviors] - spell-count trinkets")
{
    const auto scroll = FindTrinketBehavior("BG32_MagicItem_930");
    CHECK(scroll.effect == TrinketEffect::SPELL_COUNT_RANDOM_NAGA);
    CHECK(scroll.race == Race::NAGA);
    CHECK(scroll.value == 7);
    const auto wand = FindTrinketBehavior("BG36_MagicItem_307");
    CHECK(wand.effect == TrinketEffect::SPELL_COUNT_GOLD_ON_MINION);
    CHECK(wand.value == 3);
    CHECK(wand.amount == 1);
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
    CHECK(anemone.effect == TrinketEffect::START_COMBAT_NAGA_HEALTH);
    CHECK(anemone.health == 1);
    CHECK(anemone.value == 4);
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
    CHECK(cards.size() == 118);
    CHECK(cards.contains("BG30_MagicItem_996"));
    CHECK(cards.contains("BG30_MagicItem_841"));
    CHECK(cards.contains("BG36_MagicItem_220"));
    CHECK(cards.contains("BG30_MagicItem_425"));
    CHECK(cards.contains("BG30_MagicItem_419"));
    CHECK(cards.contains("BG30_MagicItem_426"));
    CHECK(cards.contains("BG30_MagicItem_706"));
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
