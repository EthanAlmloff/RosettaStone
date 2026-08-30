#ifndef ROSETTASTONE_BATTLEGROUNDS_BLOOD_GEM_RACE_BONUS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BLOOD_GEM_RACE_BONUS_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Adds a persistent Blood Gem bonus restricted to one minion race.
class BloodGemRaceBonusTask {
 public:
  BloodGemRaceBonusTask(Race race, int attack, int health)
      : m_race(race), m_attack(attack), m_health(health) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  Race GetRace() const noexcept { return m_race; }
  int GetAttack() const noexcept { return m_attack; }
  int GetHealth() const noexcept { return m_health; }
 private:
  Race m_race = Race::INVALID;
  int m_attack = 0;
  int m_health = 0;
};
}}

#endif
