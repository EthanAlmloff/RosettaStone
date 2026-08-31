#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_CARD_TO_HAND_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_CARD_TO_HAND_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class RandomCardToHandTask {
 public:
  RandomCardToHandTask(Race race, int tier, int amount, bool magneticOnly = false, bool battlecryOnly = false) : m_race(race), m_tier(tier), m_amount(amount), m_magneticOnly(magneticOnly), m_battlecryOnly(battlecryOnly) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&);
  TaskStatus Run(Player&, Minion&, Minion&);
  Race GetRace() const noexcept { return m_race; }
  int GetTier() const noexcept { return m_tier; }
  int GetAmount() const noexcept { return m_amount; }
  bool IsMagneticOnly() const noexcept { return m_magneticOnly; }
  bool IsBattlecryOnly() const noexcept { return m_battlecryOnly; }
 private:
  Race m_race = Race::INVALID;
  int m_tier = 0;
  int m_amount = 0;
  bool m_magneticOnly = false;
  bool m_battlecryOnly = false;
};
}}
#endif
