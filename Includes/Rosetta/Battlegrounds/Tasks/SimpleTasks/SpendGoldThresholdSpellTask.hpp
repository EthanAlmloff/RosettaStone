#ifndef ROSETTASTONE_BATTLEGROUNDS_SPEND_GOLD_THRESHOLD_SPELL_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SPEND_GOLD_THRESHOLD_SPELL_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Casts a free Tavern spell once after a per-turn gold-spend threshold.
class SpendGoldThresholdSpellTask {
 public:
  SpendGoldThresholdSpellTask(int threshold, std::string spellID, int amount)
      : m_threshold(threshold), m_spellID(std::move(spellID)), m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_threshold; std::string m_spellID; int m_amount;
};
}}
#endif
