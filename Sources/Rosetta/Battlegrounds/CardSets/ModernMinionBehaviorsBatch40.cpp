#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch40.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentBeetleBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
namespace
{
void AddRavagingScorpid(std::map<std::string, CardDef>& cards,
                        const char* id, int beetleAttack, int beetleHealth,
                        int summonedBeetles)
{
    Power power;
    // Rally is the Battlegrounds event emitted after each friendly minion
    // attacks.  The persistent state is consumed when Beetles are created,
    // so this remains effective across combat and recruit transitions.
    Trigger rally{ TriggerType::RALLY };
    rally.SetTriggerSource(TriggerSource::FRIENDLY);
    rally.SetTasks({ SimpleTasks::PersistentBeetleBuffTask{
        beetleAttack, beetleHealth } });
    power.AddTrigger(std::move(rally));
    power.AddDeathrattleTask(
        SimpleTasks::SummonTask{ "BG28_603t", summonedBeetles });
    cards.emplace(id, CardDef{ std::move(power) });
}
}  // namespace

void ModernMinionBehaviorsBatch40::AddAll(
    std::map<std::string, CardDef>& cards)
{
    AddRavagingScorpid(cards, "BG36_209", 5, 5, 1);
    AddRavagingScorpid(cards, "BG36_209_G", 10, 10, 2);
}
}  // namespace RosettaStone::Battlegrounds
