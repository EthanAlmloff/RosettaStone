// Copyright (c) 2026 Hearthstone BG AI contributors

#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/Models/Battle.hpp>

#include <initializer_list>
#include <string_view>

using namespace RosettaStone;
using namespace Battlegrounds;

namespace
{
Card MakeKeywordCard(std::string_view id, int attack, int health,
                     std::initializer_list<GameTag> keywords,
                     bool golden = false)
{
    Card card;
    card.id = id;
    card.gameTags[GameTag::ATK] = attack;
    card.gameTags[GameTag::HEALTH] = health;
    if (golden)
    {
        card.normalDbfID = 1;
    }
    for (const auto keyword : keywords)
    {
        card.gameTags[keyword] = 1;
    }
    return card;
}

void AddZeroAttackDummy(Player& player)
{
    Minion dummy{ MakeKeywordCard("TEST_dummy", 0, 10, {}) };
    player.recruitField.Add(dummy);
}

void AddZeroAttackTarget(Player& player, int health)
{
    Minion target{ MakeKeywordCard("TEST_target", 0, health, {}) };
    player.recruitField.Add(target);
}
}  // namespace

TEST_CASE("[Battlegrounds : Keywords] - Reborn revives normal minion once")
{
    Player player1;
    Player player2;
    Minion reborn{ MakeKeywordCard("TEST_reborn", 2, 1,
                                   { GameTag::REBORN }) };
    player1.recruitField.Add(reborn);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 10);

    Battle battle(player1, player2);
    battle.Initialize();
    CHECK(battle.Attack());

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    const Minion& revived = battle.GetPlayer1Field()[0];
    CHECK_EQ(revived.GetCardID(), "TEST_reborn");
    CHECK_EQ(revived.GetHealth(), 1);
    CHECK_FALSE(revived.HasReborn());
    CHECK_FALSE(revived.IsDestroyed());
}

TEST_CASE("[Battlegrounds : Keywords] - Reborn revives golden minion once")
{
    Player player1;
    Player player2;
    Minion reborn{ MakeKeywordCard("TEST_reborn_G", 4, 2,
                                   { GameTag::REBORN }, true) };
    player1.recruitField.Add(reborn);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 10);

    Battle battle(player1, player2);
    battle.Initialize();
    CHECK(battle.Attack());

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    const Minion& revived = battle.GetPlayer1Field()[0];
    CHECK(revived.IsGolden());
    CHECK_EQ(revived.GetCardID(), "TEST_reborn_G");
    CHECK_EQ(revived.GetHealth(), 1);
    CHECK_FALSE(revived.HasReborn());
}

TEST_CASE("[Battlegrounds : Keywords] - Windfury gives normal minion two attacks")
{
    Player player1;
    Player player2;
    Minion windfury{ MakeKeywordCard("TEST_windfury", 2, 10,
                                    { GameTag::WINDFURY }) };
    player1.recruitField.Add(windfury);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 20);

    Battle battle(player1, player2);
    battle.Initialize();
    REQUIRE(battle.Attack());
    CHECK_EQ(battle.GetPlayer2Field()[0].GetHealth(), 18);
    CHECK_EQ(battle.GetPlayer1NextAttacker(), 0);

    REQUIRE(battle.Attack());
    CHECK_EQ(battle.GetPlayer2Field()[0].GetHealth(), 16);
}

TEST_CASE("[Battlegrounds : Keywords] - Windfury gives golden minion two attacks")
{
    Player player1;
    Player player2;
    Minion windfury{ MakeKeywordCard("TEST_windfury_G", 4, 10,
                                    { GameTag::WINDFURY }, true) };
    player1.recruitField.Add(windfury);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 20);

    Battle battle(player1, player2);
    battle.Initialize();
    REQUIRE(battle.Attack());
    REQUIRE(battle.Attack());

    CHECK_EQ(battle.GetPlayer2Field()[0].GetHealth(), 12);
}

TEST_CASE("[Battlegrounds : Keywords] - Mega Windfury gives four consecutive attacks")
{
    Player player1;
    Player player2;
    Minion megaWindfury{ MakeKeywordCard("TEST_mega_windfury", 2, 10,
                                        { GameTag::MEGA_WINDFURY }) };
    player1.recruitField.Add(megaWindfury);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 20);

    Battle battle(player1, player2);
    battle.Initialize();
    for (int i = 0; i < 4; ++i)
    {
        REQUIRE(battle.Attack());
    }

    CHECK_EQ(battle.GetPlayer2Field()[0].GetHealth(), 12);
}

TEST_CASE("[Battlegrounds : Keywords] - Reborn preserves combat state and is consumed")
{
    Player player1;
    Player player2;
    Minion reborn{ MakeKeywordCard(
        "TEST_reborn_state", 2, 2,
        { GameTag::REBORN, GameTag::TAUNT, GameTag::WINDFURY }) };
    player1.recruitField.Add(reborn);
    AddZeroAttackDummy(player1);
    Minion target{ MakeKeywordCard("TEST_target", 7, 10, {}) };
    player2.recruitField.Add(target);

    Battle battle(player1, player2);
    battle.Initialize();
    battle.GetPlayer1Field()[0].SetAttack(5);
    REQUIRE(battle.Attack());

    REQUIRE_EQ(battle.GetPlayer1Field().GetCount(), 2);
    const Minion& revived = battle.GetPlayer1Field()[0];
    CHECK_EQ(revived.GetAttack(), 5);
    CHECK_EQ(revived.GetHealth(), 1);
    CHECK(revived.HasTaunt());
    CHECK(revived.HasWindfury());
    CHECK_FALSE(revived.HasReborn());

    battle.GetPlayer1Field()[0].TakeDamage(100);
    battle.ProcessDestroy(true);
    CHECK_EQ(battle.GetPlayer1Field().GetCount(), 1);
}

TEST_CASE("[Battlegrounds : Keywords] - Venomous destroys a damaged target")
{
    Player player1;
    Player player2;
    Minion venomous{ MakeKeywordCard("TEST_venomous", 1, 10,
                                    { GameTag::VENOMOUS }) };
    player1.recruitField.Add(venomous);
    AddZeroAttackDummy(player1);
    AddZeroAttackTarget(player2, 20);

    Battle battle(player1, player2);
    battle.Initialize();
    REQUIRE(battle.Attack());

    CHECK_EQ(battle.GetPlayer2Field().GetCount(), 0);
}
