#ifndef ROSETTASTONE_BATTLEGROUNDS_REBORN_ATTACK_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_REBORN_ATTACK_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class RebornAttackTask {
 public:
    explicit RebornAttackTask(int multiplier, bool rightmostUndead)
        : m_multiplier(multiplier), m_rightmostUndead(rightmostUndead) {}
    TaskStatus Run(Player&, Minion& source);
    TaskStatus Run(Player&, Minion& source, Minion& target);
 private: int m_multiplier = 1; bool m_rightmostUndead = false;
}; } }
#endif
