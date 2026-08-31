#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHandStatsTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatHandStatsTask::Run(Player& p, Minion& source) {
    int attack = 0, health = 0;
    p.hand.ForEach([&](const std::optional<CardData>& entry) {
        if (std::holds_alternative<Minion>(*entry)) {
            const auto& minion = std::get<Minion>(*entry);
            attack += minion.GetAttack(); health += minion.GetHealth();
        }
    });
    source.SetAttack(source.GetAttack() + attack * m_repeats);
    source.SetHealth(source.GetHealth() + health * m_repeats);
    return TaskStatus::COMPLETE;
}
TaskStatus StartCombatHandStatsTask::Run(Player& p, Minion& source, Minion&) { return Run(p, source); }
}
