#ifndef ROSETTASTONE_BATTLEGROUNDS_ADD_TAVERN_COIN_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ADD_TAVERN_COIN_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Adds canonical BG28_810 Tavern Coin spells to the player's hand.
class AddTavernCoinTask
{
 public:
    explicit AddTavernCoinTask(int amount);

    TaskStatus Run(Player& player, Minion& source);
    TaskStatus Run(Player& player, Minion& source, Minion& target);

    int Amount() const noexcept { return m_amount; }

 private:
    int m_amount = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
