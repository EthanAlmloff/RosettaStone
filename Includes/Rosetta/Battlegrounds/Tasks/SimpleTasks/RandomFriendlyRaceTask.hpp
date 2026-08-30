// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_FRIENDLY_RACE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_FRIENDLY_RACE_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

#include <string_view>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Applies one of the small random-friendly-race deathrattle effects.
//!
//! This task deliberately owns only the two effects currently needed by the
//! verified minion batch: a random friendly race receives a stat bonus, or a
//! random friendly race receives Reborn. Keeping the race filter here avoids
//! silently applying a targeted effect to the whole board.
class RandomFriendlyRaceTask
{
 public:
    //! Construct a random friendly race stat effect.
    RandomFriendlyRaceTask(Race race, int attack, int health, int amount = 1);

    //! Construct a random friendly race Reborn effect.
    RandomFriendlyRaceTask(Race race, int amount);

    //! Run the task against the source minion.
    TaskStatus Run(Player& player, Minion& source);

    //! Run the task against an explicit target (unused for this task).
    TaskStatus Run(Player& player, Minion& source, Minion& target);

 private:
    Race m_race = Race::INVALID;
    int m_attack = 0;
    int m_health = 0;
    int m_amount = 1;
    bool m_grantReborn = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_RANDOM_FRIENDLY_RACE_TASK_HPP
