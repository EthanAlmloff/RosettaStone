#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
#include <utility>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class CastTavernSpellTask {
 public:
  CastTavernSpellTask(std::string cardID, int amount): m_cardID(std::move(cardID)), m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  const std::string& CardID() const noexcept { return m_cardID; }
  int Amount() const noexcept { return m_amount; }
 private:
  std::string m_cardID;
  int m_amount;
};
}}
