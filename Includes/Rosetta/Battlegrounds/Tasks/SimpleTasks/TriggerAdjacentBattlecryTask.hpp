#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Player;
class Minion;

namespace SimpleTasks
{
//! Triggers the battlecries of the surviving neighbours of a removed minion.
//! The source's last field position is retained through deathrattle dispatch,
//! so this remains correct after the dead slot is removed.
class TriggerAdjacentBattlecryTask
{
 public:
    explicit TriggerAdjacentBattlecryTask(bool golden = false)
        : m_golden(golden) {}

    bool Golden() const noexcept { return m_golden; }

    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);

 private:
    bool m_golden = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds
