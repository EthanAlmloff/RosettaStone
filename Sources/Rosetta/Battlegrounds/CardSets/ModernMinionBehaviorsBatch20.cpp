#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch20.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ExactCopyDeathrattleTask.hpp>
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

    // Timewarped Radio Star: on death, copy the enemy minion whose attack
    // killed it, retaining the killer's current stats and enchantments.  The
    // combat cleanup pass arms the snapshot immediately before deathrattles
    // run; the task only consumes that authoritative snapshot.
    Power radio;
    radio.AddDeathrattleTask(SimpleTasks::ExactCopyDeathrattleTask{0, 1, true});
    cards.emplace("BG34_Giant_330", CardDef{ std::move(radio) });
    Power radioGolden;
    radioGolden.AddDeathrattleTask(SimpleTasks::ExactCopyDeathrattleTask{0, 2, true});
    cards.emplace("BG34_Giant_330_G", CardDef{ std::move(radioGolden) });
}
}
