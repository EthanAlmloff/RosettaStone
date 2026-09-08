#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Discards one spell from hand at recruit-start and grants the stated gold.
//! The hand is scanned in stable slot order to keep seeded simulations replayable.
class DiscardSpellGainGoldTask {
 public:
  explicit DiscardSpellGainGoldTask(int amount): m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int Amount() const noexcept { return m_amount; }
 private:
  int m_amount;
};
}
}
