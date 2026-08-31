#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion;
namespace SimpleTasks {
class EndTurnStatTransferTask {
 public:
  EndTurnStatTransferTask(int attack, int health, bool handTarget, int repeats = 0) : m_attack(attack), m_health(health), m_handTarget(handTarget), m_repeats(repeats) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack, m_health; bool m_handTarget; int m_repeats;
};
}}
