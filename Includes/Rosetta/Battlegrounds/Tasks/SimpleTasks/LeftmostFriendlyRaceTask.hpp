// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_LEFTMOST_FRIENDLY_RACE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_LEFTMOST_FRIENDLY_RACE_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Applies a combat-only stat/keyword grant to the left-most friendly
//! minions of a race.  The task deliberately operates on the copied combat
//! field, so recruit stats remain unchanged after the battle.
class LeftmostFriendlyRaceTask
{
 public:
    LeftmostFriendlyRaceTask(Race race, int attack, int health, int amount,
                             bool grantWindfury);

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

 private:
    Race m_race = Race::INVALID;
    int m_attack = 0;
    int m_health = 0;
    int m_amount = 1;
    bool m_grantWindfury = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_LEFTMOST_FRIENDLY_RACE_TASK_HPP
