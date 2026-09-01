#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Gives the source +amount/+amount for each friendly Golden minion.
class EndTurnGoldenCountSelfBuffTask {
 public:
  explicit EndTurnGoldenCountSelfBuffTask(int amount = 0) : m_amount(amount) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
  int Amount() const noexcept { return m_amount; }
 private: int m_amount = 0;
};
}}
