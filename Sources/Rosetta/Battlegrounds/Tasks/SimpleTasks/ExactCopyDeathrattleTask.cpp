#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ExactCopyDeathrattleTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ExactCopyDeathrattleTask::Run(Player& p, Minion&) {
    auto snapshot = p.season14.TakeExactCopyDeathrattle(m_snapshotId);
    if (!snapshot || !p.isInCombat) return TaskStatus::STOP;
    if (p.GetField().IsFull()) {
        p.ApplySummonOverflowTrinkets();
        return TaskStatus::STOP;
    }
    if (m_count <= 0) return TaskStatus::STOP;
    for (int i = 0; i < m_count; ++i) {
        if (p.GetField().IsFull()) {
            p.ApplySummonOverflowTrinkets();
            break;
        }
        Minion copy{*snapshot};
        // Radio Star copies the killer with full Health while preserving the
        // killer's current enchantments. Other exact-copy callers retain the
        // captured current-health value.
        p.ApplyFreshMinionModifiers(copy);
        if (m_fullHealth) copy.SetHealth(copy.GetMaxHealth());
        copy.getPlayerCallback = [&p]() -> Player& { return p; }; if (p.getNextCardIndexCallback) copy.SetIndex(p.getNextCardIndexCallback()); p.GetField().Add(copy);
        Minion& summoned = p.GetField()[p.GetField().GetCount() - 1];
        p.GetField().ForEachAlive([&summoned](MinionData& alive) {
            alive.value().ActivateTrigger(TriggerType::SUMMON, summoned);
        });
        p.ApplySummonTrinkets(summoned);
    }
    return TaskStatus::COMPLETE;
}
TaskStatus ExactCopyDeathrattleTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
