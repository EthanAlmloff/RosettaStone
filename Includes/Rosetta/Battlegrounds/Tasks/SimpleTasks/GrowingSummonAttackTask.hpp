#ifndef ROSETTASTONE_BATTLEGROUNDS_GROWING_SUMMON_ATTACK_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_GROWING_SUMMON_ATTACK_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class GrowingSummonAttackTask {
 public:
  GrowingSummonAttackTask(Race race, int initialAttack, int increment)
      : m_race(race), m_initialAttack(initialAttack), m_increment(increment), m_nextAttack(initialAttack) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion& owner, Minion& target);
  Race GetRace() const { return m_race; }
  int GetInitialAttack() const { return m_initialAttack; }
  int GetIncrement() const { return m_increment; }
 private:
  Race m_race = Race::INVALID;
  int m_initialAttack = 0;
  int m_increment = 0;
  int m_nextAttack = 0;
};
}}
#endif
