#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;

namespace SimpleTasks {
//! Gives the owner a Windfall Tornado at recruit-turn end.
//! The base portrait bonus is scaled by minions sold during this turn.
class EndTurnWindfallPortraitTask {
 public:
  explicit EndTurnWindfallPortraitTask(int bonus): m_bonus(bonus) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int Bonus() const noexcept { return m_bonus; }
 private:
  int m_bonus;
};
}
}
