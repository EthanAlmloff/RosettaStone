#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Player;
class Minion;

namespace SimpleTasks
{
//! Faceless Manipulator's targeted Battlecry: replace the source with the
//! target's card definition while keeping the source entity/index in its
//! existing zone. Golden Faceless upgrades the copied identity to golden.
class CopyTargetBattlecryTask
{
 public:
    explicit CopyTargetBattlecryTask(bool golden = false) : m_golden(golden) {}

    bool Golden() const noexcept { return m_golden; }

    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);

 private:
    bool m_golden = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds
