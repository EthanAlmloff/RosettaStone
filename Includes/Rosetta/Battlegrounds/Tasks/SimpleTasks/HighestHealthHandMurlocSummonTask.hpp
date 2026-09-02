#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Summons the highest-health Murlocs from hand into the combat copy.
class HighestHealthHandMurlocSummonTask
{
 public:
    explicit HighestHealthHandMurlocSummonTask(int count) : m_count(count) {}
    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);
    int Count() const noexcept { return m_count; }

 private:
    int m_count;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds
