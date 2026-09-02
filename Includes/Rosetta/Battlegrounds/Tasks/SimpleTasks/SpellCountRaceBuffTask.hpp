#ifndef ROSETTASTONE_BATTLEGROUNDS_SPELL_COUNT_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SPELL_COUNT_RACE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Deathrattle race aura whose base stats improve once per N Tavern spells.
class SpellCountRaceBuffTask {
 public:
  SpellCountRaceBuffTask(Race race, int attack, int health, int spellsPerImprovement,
                         bool selfBuff = false)
      : m_race(race), m_attack(attack), m_health(health), m_threshold(spellsPerImprovement), m_selfBuff(selfBuff) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: Race m_race; int m_attack; int m_health; int m_threshold; bool m_selfBuff = false;
};
}}
#endif
