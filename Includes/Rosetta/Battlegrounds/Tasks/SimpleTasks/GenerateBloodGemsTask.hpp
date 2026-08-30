#ifndef ROSETTASTONE_BATTLEGROUNDS_GENERATE_BLOOD_GEMS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_GENERATE_BLOOD_GEMS_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Adds canonical BG20_GEM spells to the source minion's owner's hand.
class GenerateBloodGemsTask
{
 public:
    explicit GenerateBloodGemsTask(int amount);

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

    int Amount() const noexcept { return m_amount; }

 private:
    int m_amount = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
