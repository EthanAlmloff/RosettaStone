#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthEnemyDamageTask.hpp>

#include <effolkronium/random.hpp>

#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds::SimpleTasks
{
TaskStatus HighestHealthEnemyDamageTask::Run(Player& player, Minion&)
{
    if (m_baseDamage < 0 || m_repeats <= 0 || m_damagePerSpell < 0)
        return TaskStatus::STOP;

    Player& opponent = player.getOpponentPlayerCallback(player);
    std::vector<Minion*> candidates;
    int highestHealth = -1;
    opponent.GetField().ForEachAlive([&](MinionData& data) {
        Minion& minion = data.value();
        if (minion.GetHealth() > highestHealth)
        {
            highestHealth = minion.GetHealth();
            candidates.clear();
            candidates.push_back(&minion);
        }
        else if (minion.GetHealth() == highestHealth)
        {
            candidates.push_back(&minion);
        }
    });
    if (candidates.empty())
        return TaskStatus::STOP;

    Minion& target = *candidates[Random::get<std::size_t>(0, candidates.size() - 1)];
    const int damage = m_baseDamage +
                       player.season14.SuccessfulSpellCount() * m_damagePerSpell;
    for (int repeat = 0; repeat < m_repeats; ++repeat)
        target.TakeDamage(damage);
    return TaskStatus::COMPLETE;
}

TaskStatus HighestHealthEnemyDamageTask::Run(Player& player, Minion& source,
                                             Minion&)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
