#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
#include <utility>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class CastTavernSpellOnAdjacentTask {
 public:
  CastTavernSpellOnAdjacentTask(std::string cardID, int amount):m_cardID(std::move(cardID)),m_amount(amount){}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: std::string m_cardID; int m_amount;
};
}}
