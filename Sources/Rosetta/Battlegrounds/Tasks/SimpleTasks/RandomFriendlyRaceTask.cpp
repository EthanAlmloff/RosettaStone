// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds::SimpleTasks
{
RandomFriendlyRaceTask::RandomFriendlyRaceTask(Race race, int attack,
                                               int health, int amount,
                                               bool grantDivineShield,
                                               bool improveFutureLobsters)
    : m_race(race),
      m_attack(attack),
      m_health(health),
      m_amount(amount),
      m_grantReborn(false),
      m_grantDivineShield(grantDivineShield),
      m_improveFutureLobsters(improveFutureLobsters)
{
}

RandomFriendlyRaceTask::RandomFriendlyRaceTask(Race race, int amount)
    : m_race(race), m_amount(amount), m_grantReborn(true)
{
}

TaskStatus RandomFriendlyRaceTask::Run(Player& player, Minion& source)
{
    if (m_improveFutureLobsters)
    {
        player.season14.ImproveFutureLobsters(m_attack, m_health);
    }

    std::vector<Minion*> candidates;
    player.GetField().ForEachAlive(
        [&candidates, &source, this](MinionData& minionData) {
            Minion& candidate = minionData.value();
            if (&candidate != &source && candidate.HasRace(m_race))
            {
                candidates.emplace_back(&candidate);
            }
        });

    if (candidates.empty() || m_amount <= 0)
    {
        return candidates.empty() ? TaskStatus::STOP : TaskStatus::COMPLETE;
    }

    Random::shuffle(candidates.begin(), candidates.end());
    const auto count = std::min<std::size_t>(
        static_cast<std::size_t>(m_amount), candidates.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        Minion& target = *candidates[i];
        if (m_grantReborn)
        {
            target.SetReborn(true);
        }
        else
        {
            target.SetAttack(target.GetAttack() + m_attack);
            target.SetHealth(target.GetHealth() + m_health);
            if (m_grantDivineShield)
            {
                target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            }
        }
    }

    return TaskStatus::COMPLETE;
}

TaskStatus RandomFriendlyRaceTask::Run(Player& player, Minion& source,
                                       [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
