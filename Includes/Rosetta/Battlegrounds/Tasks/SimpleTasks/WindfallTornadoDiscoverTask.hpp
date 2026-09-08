#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;

namespace SimpleTasks {
//! Opens an Elemental Discover when Windfall Tornado is sold.
//! The selected card is resolved by Player::SelectDecision so the modal
//! remains authoritative/replayable and can carry the sold instance stats.
class WindfallTornadoDiscoverTask {
 public:
  explicit WindfallTornadoDiscoverTask(int choices): m_choices(choices) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int Choices() const noexcept { return m_choices; }
 private:
  int m_choices;
};
}
}
