#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_ENEMY_DAMAGE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_ENEMY_DAMAGE_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Deals damage to one uniformly selected living enemy minion.
class RandomEnemyDamageTask {
 public:
    explicit RandomEnemyDamageTask(int damage, int count = 1)
        : m_damage(damage), m_count(count) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
    int m_damage = 0;
    int m_count = 1;
};
}}

#endif
