#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_CARD_TO_HAND_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_CARD_TO_HAND_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class RandomCardToHandTask {
 public:
  RandomCardToHandTask(Race race, int tier, int amount) : m_race(race), m_tier(tier), m_amount(amount) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  Race GetRace() const noexcept { return m_race; }
  int GetTier() const noexcept { return m_tier; }
  int GetAmount() const noexcept { return m_amount; }
 private:
  Race m_race = Race::INVALID;
  int m_tier = 0;
  int m_amount = 0;
};
}}
#endif
