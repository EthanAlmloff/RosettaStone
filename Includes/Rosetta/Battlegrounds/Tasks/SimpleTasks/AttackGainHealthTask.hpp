#ifndef ROSETTASTONE_BATTLEGROUNDS_ATTACK_GAIN_HEALTH_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ATTACK_GAIN_HEALTH_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class AttackGainHealthTask { public:
  AttackGainHealthTask(int health, Race sourceRace) : m_health(health), m_sourceRace(sourceRace) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion& source);
  int GetHealth() const { return m_health; }
 private: int m_health = 0; Race m_sourceRace = Race::INVALID;
}; } }
#endif
