#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch19.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddTavernCoinTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageHeroTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch19::AddAll(std::map<std::string, CardDef>& cards)
{
    auto add = [&cards](const char* id, TriggerType type, int amount) {
        Power power;
        Trigger trigger{ type };
        trigger.SetTriggerSource(TriggerSource::SELF);
        trigger.SetTasks(std::vector<TaskType>{
            SimpleTasks::AddTavernCoinTask{ amount } });
        power.AddTrigger(std::move(trigger));
        cards.emplace(id, CardDef{ std::move(power) });
    };
    add("BG21_017", TriggerType::DEATH, 1);
    add("BG21_017_G", TriggerType::DEATH, 2);
    add("BG33_315", TriggerType::RALLY, 1);
    add("BG33_315_G", TriggerType::RALLY, 2);
    add("BG34_234", TriggerType::TURN_START, 2);
    add("BG34_234_G", TriggerType::TURN_START, 4);
    Power normal;
    Trigger normalTrigger{ TriggerType::TURN_END };
    normalTrigger.SetTriggerSource(TriggerSource::SELF);
    normalTrigger.SetTasks(std::vector<TaskType>{
        SimpleTasks::DamageHeroTask{ 1 },
        SimpleTasks::AddTavernCoinTask{ 1 } });
    normal.AddTrigger(std::move(normalTrigger));
    cards.emplace("BG27_011", CardDef{ std::move(normal) });
    Power golden;
    Trigger trigger{ TriggerType::TURN_END };
    trigger.SetTriggerSource(TriggerSource::SELF);
    trigger.SetTasks(std::vector<TaskType>{ SimpleTasks::DamageHeroTask{ 1 },
                                            SimpleTasks::AddTavernCoinTask{ 1 },
                                            SimpleTasks::DamageHeroTask{ 1 },
                                            SimpleTasks::AddTavernCoinTask{ 1 } });
    golden.AddTrigger(std::move(trigger));
    cards.emplace("BG27_011_G", CardDef{ std::move(golden) });
}
}
