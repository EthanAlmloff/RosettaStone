#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class MagnetizationCombatBuffTask {
 public:
  MagnetizationCombatBuffTask(int base, int per):m_base(base),m_per(per){}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_base; int m_per;
};
}}
