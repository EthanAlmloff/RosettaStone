#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHandSelfCopyTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatHandSelfCopyTask::Run(Player& p, Minion& source) {
    if (p.GetField().IsFull()) return TaskStatus::STOP;
    const auto id = source.GetCardID(); const Minion* snapshot = nullptr;
    p.hand.ForEach([&](const std::optional<CardData>& entry) { if (!snapshot && std::holds_alternative<Minion>(*entry) && std::get<Minion>(*entry).GetCardID() == id) snapshot = &std::get<Minion>(*entry); });
    if (!snapshot) return TaskStatus::STOP;
    Minion copy{*snapshot}; if (m_golden) { copy.SetAttack(copy.GetAttack() * 2); copy.SetHealth(copy.GetHealth() * 2); }
    p.ApplyFreshMinionModifiers(copy); copy.getPlayerCallback = [&p]() -> Player& { return p; }; if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback()); p.GetField().Add(copy); return TaskStatus::COMPLETE;
}
TaskStatus StartCombatHandSelfCopyTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
