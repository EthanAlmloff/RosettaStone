#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class EndTurnDestroyAdjacentCopyTask {
 public:
  explicit EndTurnDestroyAdjacentCopyTask(bool both):m_both(both){}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  bool Both() const { return m_both; }
 private: bool m_both;
};
}}
