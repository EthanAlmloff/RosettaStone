#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class BuyTavernSpellMurlocTask {
 public:
  explicit BuyTavernSpellMurlocTask(int limit): m_limit(limit) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_limit;
};
}}
