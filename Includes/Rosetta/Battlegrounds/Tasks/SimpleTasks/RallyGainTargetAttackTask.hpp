#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_GAIN_TARGET_ATTACK_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_GAIN_TARGET_ATTACK_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
class RallyGainTargetAttackTask
{
public:
    explicit RallyGainTargetAttackTask(int multiplier) : m_multiplier(multiplier) {}

    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);

private:
    int m_multiplier;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
