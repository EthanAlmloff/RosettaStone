#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
//! Applies the source's carried Magnetic count as a combat-only stat bonus.
class MagnetizationSelfBuffTask {
 public:
  MagnetizationSelfBuffTask(int attack, int health): m_attack(attack), m_health(health) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack; int m_health;
};
}}
