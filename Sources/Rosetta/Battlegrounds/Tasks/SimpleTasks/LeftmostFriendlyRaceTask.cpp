// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeftmostFriendlyRaceTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
LeftmostFriendlyRaceTask::LeftmostFriendlyRaceTask(
    Race race, int attack, int health, int amount, bool grantWindfury)
    : m_race(race),
      m_attack(attack),
      m_health(health),
      m_amount(amount),
      m_grantWindfury(grantWindfury)
{
}

TaskStatus LeftmostFriendlyRaceTask::Run(Player& player, Minion& source)
{
    int remaining = m_amount;
    player.GetField().ForEachAlive(
        [this, &remaining](MinionData& minionData) {
            if (remaining <= 0)
            {
                return;
            }

            Minion& minion = minionData.value();
            if (!minion.HasRace(m_race))
            {
                return;
            }

            minion.SetAttack(minion.GetAttack() + m_attack);
            minion.SetHealth(minion.GetHealth() + m_health);
            if (m_grantWindfury)
            {
                minion.SetGameTag(GameTag::WINDFURY, 1);
            }
            --remaining;
        });

    return TaskStatus::COMPLETE;
}

TaskStatus LeftmostFriendlyRaceTask::Run(
    Player& player, Minion& source, [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
