#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Each friendly Demon consumes one Tavern minion and gains its stats.
//! The multiplier is the golden scaling of the effect, not an extra trigger.
class ConsumeTavernForFriendlyDemonsTask {
 public:
  explicit ConsumeTavernForFriendlyDemonsTask(int multiplier = 1) : m_multiplier(multiplier) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
  int Multiplier() const noexcept { return m_multiplier; }
 private: int m_multiplier = 1;
};
}}
