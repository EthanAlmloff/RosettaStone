#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyHighestHandSummonTask.hpp>

#include <algorithm>
#include <vector>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyHighestHandSummonTask::Run(Player& p, Minion&) {
    std::vector<const Minion*> candidates;
    p.hand.ForEach([&](const std::optional<CardData>& e) {
        if (std::holds_alternative<Minion>(*e))
            candidates.push_back(&std::get<Minion>(*e));
    });
    // stable_sort makes equal-attack ties resolve in hand order, while the
    // prefix gives the golden card two distinct highest-Attack instances.
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Minion* left, const Minion* right) {
                         return left->GetAttack() > right->GetAttack();
                     });
    const auto count = std::min<std::size_t>(
        candidates.size(), static_cast<std::size_t>(std::max(0, m_count)));
    if (count == 0)
        return TaskStatus::STOP;
    for (std::size_t i = 0; i < count; ++i)
        p.season14.ArmCombatHandSummon(*candidates[i], 1);
    return TaskStatus::COMPLETE;
}
TaskStatus RallyHighestHandSummonTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
