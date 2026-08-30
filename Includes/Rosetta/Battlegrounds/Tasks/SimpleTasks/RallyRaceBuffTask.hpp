#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_RACE_BUFF_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Applies a permanent stat bonus to every living friendly minion of a race.
class RallyRaceBuffTask {
 public:
  RallyRaceBuffTask(Race race, int attack, int health,
                    Race triggerRace = Race::INVALID)
      : m_race(race), m_attack(attack), m_health(health),
        m_triggerRace(triggerRace) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  Race m_race = Race::INVALID;
  int m_attack = 0;
  int m_health = 0;
  Race m_triggerRace = Race::INVALID;
};
}}
#endif
