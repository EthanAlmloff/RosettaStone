#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch24.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds {
namespace { void AddBattlecry(std::map<std::string,CardDef>& c,const char* id,const char* card,int n){Power p;p.AddBattlecryTask(SimpleTasks::AddCardTask{card,n});c.emplace(id,CardDef{std::move(p)});} void AddBoth(std::map<std::string,CardDef>& c,const char* id,const char* card,int n){Power p;p.AddBattlecryTask(SimpleTasks::AddCardTask{card,n});p.AddDeathrattleTask(SimpleTasks::AddCardTask{card,n});c.emplace(id,CardDef{std::move(p)});} void AddDeath(std::map<std::string,CardDef>& c,const char* id,const char* card,int n){Power p;p.AddDeathrattleTask(SimpleTasks::AddCardTask{card,n});c.emplace(id,CardDef{std::move(p)});} }
void ModernMinionBehaviorsBatch24::AddAll(std::map<std::string,CardDef>& cards){
 AddBattlecry(cards,"BG31_822","BG31_891",1); AddBattlecry(cards,"BG31_822_G","BG31_891",2);
 AddBoth(cards,"BG32_111","BG28_888",1); AddBoth(cards,"BG32_111_G","BG28_888",2);
 AddDeath(cards,"BG32_170","EBG_Spell_014",1); AddDeath(cards,"BG32_170_G","EBG_Spell_014",2);
 AddBoth(cards,"BG32_336","BG32_337",1); AddBoth(cards,"BG32_336_G","BG32_337",2);
 AddBoth(cards,"BG32_820","BG28_168",1); AddBoth(cards,"BG32_820_G","BG28_168",2);
}
}
