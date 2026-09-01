#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHighestHandMinionSummonTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatHighestHandMinionSummonTask::Run(Player& p, Minion&) {
    if (p.GetField().IsFull()) return TaskStatus::STOP;
    const Minion* best = nullptr;
    p.hand.ForEach([&](const std::optional<CardData>& entry) {
        if (!entry || !std::holds_alternative<Minion>(*entry)) return;
        const auto& candidate = std::get<Minion>(*entry);
        if (!best || candidate.GetHealth() > best->GetHealth()) best = &candidate;
    });
    if (!best) return TaskStatus::STOP;
    Minion copy{*best};
    copy.SetAttack(copy.GetAttack() + m_attack);
    copy.SetHealth(copy.GetHealth() + m_health);
    p.ApplyFreshMinionModifiers(copy);
    copy.getPlayerCallback = [&p]() -> Player& { return p; };
    if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback());
    p.GetField().Add(copy);
    Minion& summoned = p.GetField()[p.GetField().GetCount() - 1];
    p.GetField().ForEachAlive([&summoned](MinionData& data) {
        data.value().ActivateTrigger(TriggerType::SUMMON, summoned);
    });
    p.ApplySummonTrinkets(summoned);
    return TaskStatus::COMPLETE;
}

TaskStatus StartCombatHighestHandMinionSummonTask::Run(Player& p, Minion& s, Minion&) {
    return Run(p, s);
}
} // namespace RosettaStone::Battlegrounds::SimpleTasks
