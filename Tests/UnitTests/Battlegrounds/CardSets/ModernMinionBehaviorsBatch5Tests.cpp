// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Games/Game.hpp>
#include <Rosetta/Battlegrounds/Models/Battle.hpp>

#include <map>
#include <string>

using namespace RosettaStone;
using namespace Battlegrounds;

namespace
{
void AddCard(Player& player, const char* id)
{
    Minion minion{ Cards::FindCardByID(id) };
    player.recruitField.Add(minion);
}

void PreparePlayers(Player& player1, Player& player2)
{
    player1.isInCombat = false;
    player2.isInCombat = false;
    AddCard(player1, "BG25_022");
    AddCard(player1, "BG25_001");
    AddCard(player2, "BGS_039");
}

void KillFirstMinion(Battle& battle)
{
    battle.GetPlayer1Field()[0].TakeDamage(100);
    battle.ProcessDestroy(true);
}
}  // namespace

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - batch 5 registrations")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch5::AddAll(cards);

    for (const auto* id : { "BG25_022", "BG25_022_G", "BG28_309",
                            "BG28_309_G" })
    {
        REQUIRE(cards.contains(id));
        CHECK(cards.at(id).power.GetBattlecryTask().empty());
        CHECK(cards.at(id).power.GetStartCombatTask().empty());
        CHECK_EQ(cards.at(id).power.GetDeathrattleTask().size(), 1);
    }
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - Scarlet Skull normal")
{
    Game game;
    game.Start();
    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    PreparePlayers(player1, player2);

    Battle battle(player1, player2);
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.Initialize();
    KillFirstMinion(battle);

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    const Minion& target = battle.GetPlayer1Field()[1];
    CHECK_EQ(target.GetCardID(), "BG25_001");
    CHECK_EQ(target.GetAttack(), 3);
    CHECK_EQ(target.GetHealth(), 3);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - Scarlet Skull golden")
{
    Game game;
    game.Start();
    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    AddCard(player1, "BG25_022_G");
    AddCard(player1, "BG25_001");
    AddCard(player2, "BGS_039");

    Battle battle(player1, player2);
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.Initialize();
    KillFirstMinion(battle);

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    const Minion& target = battle.GetPlayer1Field()[1];
    CHECK_EQ(target.GetCardID(), "BG25_001");
    CHECK_EQ(target.GetAttack(), 4);
    CHECK_EQ(target.GetHealth(), 5);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - golden Mummifier picks two distinct Undead")
{
    Game game;
    game.Start();
    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    AddCard(player1, "BG28_309_G");
    AddCard(player1, "BG25_001");
    AddCard(player1, "BG25_001");
    AddCard(player2, "BGS_039");

    Battle battle(player1, player2);
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.Initialize();
    KillFirstMinion(battle);

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    CHECK(battle.GetPlayer1Field()[0].HasReborn());
    CHECK(battle.GetPlayer1Field()[1].HasReborn());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - normal Mummifier filters race")
{
    Game game;
    game.Start();
    Player& player1 = game.GetGameState().players[0];
    Player& player2 = game.GetGameState().players[1];
    AddCard(player1, "BG28_309");
    AddCard(player1, "BG25_001");
    AddCard(player1, "BG31_803");
    AddCard(player2, "BGS_039");

    Battle battle(player1, player2);
    player1.isInCombat = true;
    player2.isInCombat = true;
    battle.Initialize();
    KillFirstMinion(battle);

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    CHECK(battle.GetPlayer1Field()[0].HasReborn());
    CHECK_FALSE(battle.GetPlayer1Field()[1].HasReborn());
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviors] - race matching keeps dual and ALL tribes")
{
    // These records are deliberately not registered behavior cards: the
    // task only needs their pinned metadata to determine tribe eligibility.
    const Minion dualTribe{ Cards::FindCardByID("BG24_005") };
    const Minion allTribes{ Cards::FindCardByID("BG27_080") };
    const Minion beastOnly{ Cards::FindCardByID("BG31_803") };

    REQUIRE_FALSE(dualTribe.GetCardID().empty());
    REQUIRE_FALSE(allTribes.GetCardID().empty());
    REQUIRE_FALSE(beastOnly.GetCardID().empty());
    CHECK(dualTribe.HasRace(Race::UNDEAD));
    CHECK(dualTribe.HasRace(Race::BEAST));
    CHECK(allTribes.HasRace(Race::UNDEAD));
    CHECK_FALSE(beastOnly.HasRace(Race::UNDEAD));
}
