#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Player;
class Minion;
namespace SimpleTasks {
//! Rally effect for Jailbird Juggernaut. The attack is deferred until Battle
//! can safely insert and resolve the summoned attacker.
class RallyBloodGemGolemTask {
 public:
  explicit RallyBloodGemGolemTask(int multiplier = 1) : m_multiplier(multiplier) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  int m_multiplier;
};
}}
