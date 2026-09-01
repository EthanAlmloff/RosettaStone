#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTrackedAvengeCardsTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SummonTrackedAvengeCardsTask::Run(Player& p, Minion&) {
    if (!p.isInCombat) return TaskStatus::STOP;
    auto ids = p.season14.TakeCombatAvengeCards();
    bool summonedAny = false;
    for (auto& id : ids) {
        for (int i = 0; i < p.hand.GetCount(); ++i) {
            if (std::holds_alternative<Minion>(p.hand[i]) &&
                std::get<Minion>(p.hand[i]).GetCardID() == id &&
                !p.GetField().IsFull()) {
                auto card = p.hand.Remove(p.hand[i]);
                auto minion = std::get<Minion>(std::move(card));
                p.GetField().Add(minion);
                Minion& summoned = p.GetField()[p.GetField().GetCount() - 1];
                p.GetField().ForEachAlive([&summoned](MinionData& alive) {
                    alive.value().ActivateTrigger(TriggerType::SUMMON, summoned);
                });
                p.ApplySummonTrinkets(summoned);
                summonedAny = true;
                break;
            }
        }
    }
    return summonedAny ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus SummonTrackedAvengeCardsTask::Run(Player& p, Minion& s, Minion&) {
    return Run(p, s);
}
}
