#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class ConsumeRandomTavernTask {
 public:
  explicit ConsumeRandomTavernTask(int multiplier = 1) : m_multiplier(multiplier) {}
  int Multiplier() const noexcept { return m_multiplier; }
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion& target);
 private: int m_multiplier = 1;
};
}}
