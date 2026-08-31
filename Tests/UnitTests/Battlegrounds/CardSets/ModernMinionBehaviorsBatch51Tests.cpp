#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch51.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <map>
#include <variant>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch51] - Chromadrake generation family") {
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch51::AddAll(cards);
    REQUIRE(cards.size() == 6);
    CHECK(cards.at("BG34_633").power.GetBattlecryTask().size() == 1);
    CHECK(cards.at("BG34_633").power.GetDeathrattleTask().size() == 1);
    CHECK(cards.at("BG34_633_G").power.GetBattlecryTask().size() == 1);
    CHECK(cards.at("BG36_240").power.GetActivate()->effect == ActivateEffect::RANDOM_CHROMADRAKE);
    CHECK(cards.at("BG36_240_G").power.GetActivate()->amount == 2);
    CHECK(cards.at("BG36_245").power.GetStartCombatTask().size() == 1);
    CHECK(cards.at("BG36_245_G").power.GetStartCombatTask().size() == 1);
    const auto& task = std::get<SimpleTasks::CastTavernSpellTask>(
        cards.at("BG36_245").power.GetStartCombatTask().front());
    CHECK(task.CardID() == "BG28_168");
    CHECK(task.Amount() == 2);
}

TEST_CASE("[Battlegrounds : ModernMinionBehaviorsBatch51] - Runic free cast does not spend gold") {
    Player player;
    player.remainCoin = 7;
    player.isInCombat = false;
    CHECK(player.CastTavernSpellFree("BG28_168", 2));
    CHECK(player.remainCoin == 7);
    CHECK(player.recruitField.GetCount() == 0);
}
