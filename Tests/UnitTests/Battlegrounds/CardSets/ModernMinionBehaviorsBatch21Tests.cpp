#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch21] - Minted Corsair registry")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch21::AddAll(cards);
    CHECK(cards.empty());
    CHECK_EQ(FindSellBehaviorBatch21("BG34_230").tavernCoins, 1);
    CHECK_EQ(FindSellBehaviorBatch21("BG34_230_G").tavernCoins, 2);
    CHECK_EQ(FindSellBehaviorBatch21("UNSUPPORTED").tavernCoins, 0);
}

TEST_CASE("[ModernMinionBehaviorsBatch21] - sell path owns canonical coin grant")
{
    // The behavior is intentionally empty: Player::SellMinion performs the
    // valid-removal/pool-return sequence, then calls AddTavernCoins so a full
    // hand fails closed and the generated card remains bridge-observable.
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch21::AddAll(cards);
    CHECK(FindSellBehaviorBatch21("BG34_230").tavernCoins == 1);
    CHECK(FindSellBehaviorBatch21("BG34_230_G").tavernCoins == 2);
    const auto coin = Cards::FindCardByID("BG28_810");
    CHECK(coin.id == "BG28_810");
    CHECK(coin.GetCardType() == CardType::BATTLEGROUND_SPELL);
}
