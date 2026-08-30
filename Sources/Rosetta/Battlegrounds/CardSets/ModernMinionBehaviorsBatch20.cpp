#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch20.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch20::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Giant Rat: Battlecry and Deathrattle: gain 1 Gold next turn.
    Power normal;
    normal.AddBattlecryTask(SimpleTasks::GainGoldTask{ 1, true });
    normal.AddDeathrattleTask(SimpleTasks::GainGoldTask{ 1, true });
    cards.emplace("BG34_Giant_001", CardDef{ normal });
    Power golden;
    golden.AddBattlecryTask(SimpleTasks::GainGoldTask{ 2, true });
    golden.AddDeathrattleTask(SimpleTasks::GainGoldTask{ 2, true });
    cards.emplace("BG34_Giant_001_G", CardDef{ std::move(golden) });
}
}
