#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion;
namespace SimpleTasks {
class RandomTavernSpellToHandTask {
 public:
  RandomTavernSpellToHandTask(int amount, int maxCost = 0,
                              bool distinct = false)
      : m_amount(amount), m_maxCost(maxCost), m_distinct(distinct) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_amount; int m_maxCost; bool m_distinct;
};
}}
