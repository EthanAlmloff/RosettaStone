#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch23.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddFixedDeathrattleSummon(std::map<std::string, CardDef>& cards,
                               const char* id, const char* token, int count)
{
    Power power;
    power.AddDeathrattleTask(SimpleTasks::SummonTask{ token, count });
    cards.emplace(id, CardDef{ std::move(power) });
}
}
void ModernMinionBehaviorsBatch23::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Exact fixed-token deathrattles; no random, conditional, or immediate
    // attack behavior is represented in this batch.
    AddFixedDeathrattleSummon(cards, "BG21_029", "BG_BRM_006t", 2);
    AddFixedDeathrattleSummon(cards, "BG21_029_G", "BG_BRM_006t", 4);
    AddFixedDeathrattleSummon(cards, "BG31_807", "BG28_603t", 3);
    AddFixedDeathrattleSummon(cards, "BG31_807_G", "BG28_603t", 6);
    AddFixedDeathrattleSummon(cards, "BG33_157", "BG_CS2_065", 2);
    AddFixedDeathrattleSummon(cards, "BG33_157_G", "BG_CS2_065", 4);
    AddFixedDeathrattleSummon(cards, "BG26_ETC_321", "BG_GVG_085", 3);
    AddFixedDeathrattleSummon(cards, "BG26_ETC_321_G", "BG_GVG_085_G", 3);
}
}
