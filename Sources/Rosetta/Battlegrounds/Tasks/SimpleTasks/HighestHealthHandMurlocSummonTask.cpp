#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthHandMurlocSummonTask.hpp>

#include <algorithm>
#include <vector>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus HighestHealthHandMurlocSummonTask::Run(Player& player, Minion&)
{
    if (player.GetField().IsFull()) return TaskStatus::STOP;
    std::vector<const Minion*> candidates;
    player.hand.ForEach([&](const std::optional<CardData>& entry) {
        if (entry && std::holds_alternative<Minion>(*entry) &&
            std::get<Minion>(*entry).HasRace(Race::MURLOC))
            candidates.push_back(&std::get<Minion>(*entry));
    });
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Minion* left, const Minion* right) {
                         return left->GetHealth() > right->GetHealth();
                     });
    const auto count = std::min<std::size_t>(
        candidates.size(), static_cast<std::size_t>(std::max(0, m_count)));
    if (count == 0) return TaskStatus::STOP;
    std::size_t summonedCount = 0;
    for (std::size_t i = 0; i < count && !player.GetField().IsFull(); ++i)
    {
        Minion copy{*candidates[i]};
        player.ApplyFreshMinionModifiers(copy);
        copy.getPlayerCallback = [&player]() -> Player& { return player; };
        if (player.getNextCardIndexCallback)
            copy.SetIndex(player.getNextCardIndexCallback());
        if (!player.SummonCombatSnapshot(std::move(copy))) break;
        player.ApplySummonTrinkets(
            player.GetField()[static_cast<std::size_t>(player.GetField().GetCount() - 1)]);
        ++summonedCount;
    }
    return summonedCount == 0 ? TaskStatus::STOP : TaskStatus::COMPLETE;
}

TaskStatus HighestHealthHandMurlocSummonTask::Run(Player& player, Minion& source,
                                                  Minion&)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
