#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHighestHandMurlocSummonTask.hpp>
#include <algorithm>
#include <vector>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatHighestHandMurlocSummonTask::Run(Player& p, Minion&) {
    if (p.GetField().IsFull()) return TaskStatus::STOP;
    std::vector<const Minion*> candidates;
    p.hand.ForEach([&](const std::optional<CardData>& entry) { if (std::holds_alternative<Minion>(*entry) && std::get<Minion>(*entry).HasRace(Race::MURLOC)) candidates.push_back(&std::get<Minion>(*entry)); });
    std::stable_sort(candidates.begin(), candidates.end(), [](const Minion* a, const Minion* b) { return a->GetAttack() > b->GetAttack(); });
    const auto count = std::min<std::size_t>(candidates.size(), static_cast<std::size_t>(std::max(0, m_count)));
    for (std::size_t i = 0; i < count && !p.GetField().IsFull(); ++i) {
        Minion copy{*candidates[i]}; p.ApplyFreshMinionModifiers(copy); copy.getPlayerCallback = [&p]() -> Player& { return p; }; if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback()); p.GetField().Add(copy);
        Minion& summoned = p.GetField()[p.GetField().GetCount() - 1];
        p.GetField().ForEachAlive([&summoned](MinionData& alive) {
            alive.value().ActivateTrigger(TriggerType::SUMMON, summoned);
        });
        p.ApplySummonTrinkets(summoned);
    }
    return count == 0 ? TaskStatus::STOP : TaskStatus::COMPLETE;
}
TaskStatus StartCombatHighestHandMurlocSummonTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
