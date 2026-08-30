#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch24.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch24] - migrated to declarative registry") { std::map<std::string,CardDef> cards; ModernMinionBehaviorsBatch24::AddAll(cards); CHECK(cards.empty()); }

TEST_CASE("[ModernMinionBehaviorsBatch24] - generated entities are Tavern spells")
{
    for (const auto id : {"BG31_891", "BG28_888", "EBG_Spell_014", "BG32_337", "BG28_168"})
    {
        const auto card = Cards::FindCardByID(id);
        REQUIRE(card.id == id);
        CHECK(card.GetCardType() == CardType::BATTLEGROUND_SPELL);
    }
}
