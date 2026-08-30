#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch24.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <map>
using namespace RosettaStone::Battlegrounds;
TEST_CASE("[ModernMinionBehaviorsBatch24] - fixed generated cards") { struct E{const char* id;const char* card;int n;int battle;int death;}; constexpr E rows[]={{"BG31_822","BG31_891",1,1,0},{"BG31_822_G","BG31_891",2,1,0},{"BG32_111","BG28_888",1,1,1},{"BG32_111_G","BG28_888",2,1,1},{"BG32_170","EBG_Spell_014",1,0,1},{"BG32_170_G","EBG_Spell_014",2,0,1},{"BG32_336","BG32_337",1,1,1},{"BG32_336_G","BG32_337",2,1,1},{"BG32_820","BG28_168",1,1,1},{"BG32_820_G","BG28_168",2,1,1}}; std::map<std::string,CardDef> cards;ModernMinionBehaviorsBatch24::AddAll(cards);CHECK_EQ(cards.size(),10);for(const auto&r:rows){REQUIRE(cards.contains(r.id));CHECK_EQ(cards.at(r.id).power.GetBattlecryTask().size(),r.battle);CHECK_EQ(cards.at(r.id).power.GetDeathrattleTask().size(),r.death);const auto& tasks=r.battle?cards.at(r.id).power.GetBattlecryTask():cards.at(r.id).power.GetDeathrattleTask();if(!tasks.empty()){REQUIRE(std::holds_alternative<SimpleTasks::AddCardTask>(tasks.front()));const auto&t=std::get<SimpleTasks::AddCardTask>(tasks.front());CHECK(t.CardID()==r.card);CHECK_EQ(t.Amount(),r.n);}}}

TEST_CASE("[ModernMinionBehaviorsBatch24] - generated entities are Tavern spells")
{
    for (const auto id : {"BG31_891", "BG28_888", "EBG_Spell_014", "BG32_337", "BG28_168"})
    {
        const auto card = Cards::FindCardByID(id);
        REQUIRE(card.id == id);
        CHECK(card.GetCardType() == CardType::BATTLEGROUND_SPELL);
    }
}
