#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_TAVERN_SPELL_HEALTH_BONUS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_TAVERN_SPELL_HEALTH_BONUS_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Adds a player-owned health bonus to all subsequently resolved Tavern spells.
class RallyTavernSpellHealthBonusTask {
 public:
  explicit RallyTavernSpellHealthBonusTask(int health) : m_health(health) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  int GetHealth() const { return m_health; }
 private:
  int m_health = 0;
};
}}
#endif
