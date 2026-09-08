#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
#include <utility>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class CastTavernSpellOnAdjacentTask {
 public:
  CastTavernSpellOnAdjacentTask(std::string cardID, int amount, bool both = false):m_cardID(std::move(cardID)),m_amount(amount),m_both(both){}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  bool Both() const { return m_both; }
 private: std::string m_cardID; int m_amount; bool m_both;
};
}}
