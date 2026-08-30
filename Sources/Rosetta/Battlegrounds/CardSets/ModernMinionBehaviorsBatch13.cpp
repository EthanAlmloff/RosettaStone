#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch13.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch13::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Shell Collector: get one Tavern Coin; golden gets two.
    Power shellCollector;
    shellCollector.AddBattlecryTask(SimpleTasks::GainGoldTask{ 1 });
    cards.emplace("BG23_002", CardDef{ shellCollector });
    shellCollector.ClearData();
    shellCollector.AddBattlecryTask(SimpleTasks::GainGoldTask{ 2 });
    cards.emplace("BG23_002_G", CardDef{ std::move(shellCollector) });

    // Southsea Busker: gain one Gold next turn; golden gets two.
    Power southseaBusker;
    southseaBusker.AddBattlecryTask(SimpleTasks::GainGoldTask{ 1, true });
    cards.emplace("BG26_135", CardDef{ southseaBusker });
    southseaBusker.ClearData();
    southseaBusker.AddBattlecryTask(SimpleTasks::GainGoldTask{ 2, true });
    cards.emplace("BG26_135_G", CardDef{ std::move(southseaBusker) });
}
}  // namespace RosettaStone::Battlegrounds
