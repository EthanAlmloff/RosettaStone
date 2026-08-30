// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Games/Game.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>

#include <initializer_list>
#include <map>
#include <string>
#include <string_view>

using namespace RosettaStone;
using namespace Battlegrounds;

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 4 registrations")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch4::AddAll(cards);

    for (const auto* id : { "BG25_001", "BG25_001_G", "TB_BaconUps_099",
                            "TB_BaconUps_159", "TB_BaconUps_251",
                            "BG25_010t", "BG25_010_Gt" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
    }

    REQUIRE(cards.contains("BG25_010"));
    REQUIRE(cards.contains("BG25_010_G"));
    CHECK_EQ(cards.at("BG25_010").power.GetDeathrattleTask().size(), 1);
    CHECK_EQ(cards.at("BG25_010_G").power.GetDeathrattleTask().size(), 1);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 4 metadata flags")
{
    // Static keyword registrations intentionally have no Power task chain;
    // CardLoader remains the source of truth for keyword tags and stats.
    const auto checkTags = [](std::string_view id,
                              std::initializer_list<GameTag> tags) {
        const Card card = Cards::FindCardByID(id);
        REQUIRE_FALSE(card.id.empty());
        for (const auto tag : tags)
        {
            CHECK(card.gameTags.contains(tag));
            CHECK_EQ(card.gameTags.at(tag), 1);
        }
    };

    checkTags("BG25_001", { GameTag::TAUNT, GameTag::REBORN });
    checkTags("BG25_001_G", { GameTag::TAUNT, GameTag::REBORN });
    checkTags("TB_BaconUps_099",
              { GameTag::MAGNETIC, GameTag::DIVINE_SHIELD, GameTag::TAUNT });
    checkTags("TB_BaconUps_159", { GameTag::DIVINE_SHIELD, GameTag::WINDFURY });
    checkTags("TB_BaconUps_251", { GameTag::VENOMOUS });
    checkTags("BG25_010t", { GameTag::REBORN });
    checkTags("BG25_010_Gt", { GameTag::REBORN });

    const auto normal = Cards::FindCardByID("BG25_010");
    const auto golden = Cards::FindCardByID("BG25_010_G");
    CHECK_EQ(normal.GetAttack(), 2);
    CHECK_EQ(normal.GetHealth(), 1);
    CHECK_EQ(golden.GetAttack(), 4);
    CHECK_EQ(golden.GetHealth(), 2);
}

TEST_CASE("[Battlegrounds : Minion] - Handless Forsaken summons normal hand")
{
    Game game;
    game.Start();

    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    player1.hero.Initialize(Cards::FindCardByDbfID(59397));
    player2.hero.Initialize(Cards::FindCardByDbfID(59397));
    game.SetPlayerPair(0, 1);

    player1.hand.Add(Minion{ Cards::FindCardByID("BG25_010") });
    player1.PlayCard(0, 0);
    player2.hand.Add(Minion{ Cards::FindCardByID("BGS_039") });
    player2.PlayCard(0, 0);

    Battle battle(player1, player2);
    battle.Initialize();
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.GetPlayer1Field()[0].TakeDamage(100);
    battle.Attack();

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 1);
    CHECK_EQ(battle.GetPlayer1Field()[0].GetCardID(), "BG25_010t");
    CHECK_EQ(battle.GetPlayer1Field()[0].GetAttack(), 2);
    CHECK_EQ(battle.GetPlayer1Field()[0].GetHealth(), 1);
}

TEST_CASE("[Battlegrounds : Minion] - golden Handless Forsaken summons two golden hands")
{
    Game game;
    game.Start();

    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    player1.hero.Initialize(Cards::FindCardByDbfID(59397));
    player2.hero.Initialize(Cards::FindCardByDbfID(59397));
    game.SetPlayerPair(0, 1);

    player1.hand.Add(Minion{ Cards::FindCardByID("BG25_010_G") });
    player1.PlayCard(0, 0);
    player2.hand.Add(Minion{ Cards::FindCardByID("BGS_039") });
    player2.PlayCard(0, 0);

    Battle battle(player1, player2);
    battle.Initialize();
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.GetPlayer1Field()[0].TakeDamage(100);
    battle.Attack();

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    CHECK_EQ(battle.GetPlayer1Field()[0].GetCardID(), "BG25_010_Gt");
    CHECK_EQ(battle.GetPlayer1Field()[1].GetCardID(), "BG25_010_Gt");
    CHECK_EQ(battle.GetPlayer1Field()[0].GetAttack(), 4);
    CHECK_EQ(battle.GetPlayer1Field()[0].GetHealth(), 2);
}
