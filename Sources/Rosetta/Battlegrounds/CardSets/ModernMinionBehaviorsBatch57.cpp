#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch57.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch57::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Player::SellMinion activates the sold entity's self-scoped
    // SELL_MINION trigger after removing it from the board.  Restricting the
    // generated pool to Murlocs and tier zero matches Tad's random Murloc
    // wording; the golden copy emits two independent offers.
    Power normal;
    Trigger sold{TriggerType::SELL_MINION};
    sold.SetTriggerSource(TriggerSource::SELF);
    sold.SetTasks({SimpleTasks::RandomCardToHandTask{
        Race::MURLOC, 0, 1, false, false}});
    normal.AddTrigger(std::move(sold));
    cards.emplace("BG22_202", CardDef{std::move(normal)});

    Power golden;
    Trigger goldenSold{TriggerType::SELL_MINION};
    goldenSold.SetTriggerSource(TriggerSource::SELF);
    goldenSold.SetTasks({SimpleTasks::RandomCardToHandTask{
        Race::MURLOC, 0, 2, false, false}});
    golden.AddTrigger(std::move(goldenSold));
    cards.emplace("BG22_202_G", CardDef{std::move(golden)});
}
}  // namespace RosettaStone::Battlegrounds
