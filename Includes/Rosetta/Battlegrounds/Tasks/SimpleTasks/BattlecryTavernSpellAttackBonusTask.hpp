#ifndef ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_TAVERN_SPELL_ATTACK_BONUS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_TAVERN_SPELL_ATTACK_BONUS_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Adds a player-owned attack bonus to stat-bearing Tavern spells.
class BattlecryTavernSpellAttackBonusTask {
 public:
  explicit BattlecryTavernSpellAttackBonusTask(int attack) : m_attack(attack) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int GetAttack() const { return m_attack; }
 private: int m_attack = 0;
};
}}
#endif
