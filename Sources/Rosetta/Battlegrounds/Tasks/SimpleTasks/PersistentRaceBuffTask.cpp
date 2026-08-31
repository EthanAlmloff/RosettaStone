#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion& source) {
    const int attack = m_outsideCombatAttack >= 0 && !player.isInCombat
                           ? m_outsideCombatAttack : m_attack;
    if (!m_excludeSource) {
        player.ApplyPersistentRaceStats(m_race, attack, m_health);
        return TaskStatus::COMPLETE;
    }
    // Apply the same player-wide persistence while withholding the source
    // instance. Future instances still receive the stored aura.
    player.season14.AddPersistentRaceStats(m_race, attack, m_health);
    player.recruitField.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &source) data.value().ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    player.tavern.fieldZone.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &source) data.value().ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    player.hand.ForEach([&](std::optional<CardData>& data) {
        if (data.has_value() && std::holds_alternative<Minion>(data.value()) &&
            &std::get<Minion>(data.value()) != &source)
            std::get<Minion>(data.value()).ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    return TaskStatus::COMPLETE;
}
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
