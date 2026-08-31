// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_END_TURN_LAST_TAVERN_SPELL_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_END_TURN_LAST_TAVERN_SPELL_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Adds copies of the last Tavern spell cast at the end of the turn.
class EndTurnLastTavernSpellTask {
 public:
  explicit EndTurnLastTavernSpellTask(int amount) : m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_amount = 0;
};
} }
#endif
