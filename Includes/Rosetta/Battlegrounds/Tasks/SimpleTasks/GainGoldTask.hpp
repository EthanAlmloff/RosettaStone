#ifndef ROSETTASTONE_BATTLEGROUNDS_GAIN_GOLD_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_GAIN_GOLD_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Grants recruit gold immediately or on the next recruit turn.
class GainGoldTask
{
 public:
    explicit GainGoldTask(int amount, bool nextTurn = false);

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

    int Amount() const noexcept { return m_amount; }
    bool IsNextTurn() const noexcept { return m_nextTurn; }

 private:
    int m_amount = 0;
    bool m_nextTurn = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
