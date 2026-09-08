#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
//! Gives each other friendly Pirate a Scallywag deathrattle at combat start.
class StartCombatPirateScallywagTask {
 public:
  StartCombatPirateScallywagTask(int repeats, bool golden): m_repeats(repeats), m_golden(golden) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  int m_repeats;
  bool m_golden;
};
}
}
