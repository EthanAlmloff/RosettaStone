#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatSpellScaledRaceBuffTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus StartCombatSpellScaledRaceBuffTask::Run(Player& player, Minion& source)
{
    if (m_attack < 0 || m_health < 0 || m_improvementPerSpell < 0)
        return TaskStatus::STOP;
    // Start-of-combat tasks run against Player::battleField.  The source
    // instance carries the lifetime counter copied from recruitField.
    const int scaled = source.StartCombatSpellImprovement() * m_improvementPerSpell;
    player.GetField().ForEachAlive([&](MinionData& data) {
        auto& minion = data.value();
        if (minion.HasRace(m_race))
        {
            minion.SetAttack(minion.GetAttack() + m_attack + scaled);
            minion.SetHealth(minion.GetHealth() + m_health + scaled);
        }
    });
    return TaskStatus::COMPLETE;
}

TaskStatus StartCombatSpellScaledRaceBuffTask::Run(Player& player, Minion& source,
                                                   Minion&)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
