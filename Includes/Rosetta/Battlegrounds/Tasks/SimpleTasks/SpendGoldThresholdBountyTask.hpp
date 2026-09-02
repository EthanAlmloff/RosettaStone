#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class SpendGoldThresholdBountyTask {
 public:
  SpendGoldThresholdBountyTask(int threshold, int amount): m_threshold(threshold), m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  int m_threshold;
  int m_amount;
};
}}
