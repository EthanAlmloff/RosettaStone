#ifndef ROSETTASTONE_BATTLEGROUNDS_SUMMONED_MINION_STAT_MULTIPLIER_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SUMMONED_MINION_STAT_MULTIPLIER_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class SummonedMinionStatMultiplierTask {
 public:
  SummonedMinionStatMultiplierTask(Race race, int attackMultiplier, int healthMultiplier = 1)
      : m_race(race), m_attackMultiplier(attackMultiplier), m_healthMultiplier(healthMultiplier) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion& target);
  Race GetRace() const { return m_race; }
  int GetAttackMultiplier() const { return m_attackMultiplier; }
  int GetHealthMultiplier() const { return m_healthMultiplier; }
 private:
  Race m_race = Race::INVALID;
  int m_attackMultiplier = 1;
  int m_healthMultiplier = 1;
};
}}
#endif
