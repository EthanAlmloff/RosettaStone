#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_SUMMON_FROM_POOL_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_SUMMON_FROM_POOL_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class RandomSummonFromPoolTask {
 public:
  RandomSummonFromPoolTask(Race race, int minTier, int maxTier, int stat, bool golden)
      : m_race(race), m_minTier(minTier), m_maxTier(maxTier), m_stat(stat), m_golden(golden) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private:
  Race m_race = Race::INVALID; int m_minTier = 0; int m_maxTier = 0; int m_stat = 0; bool m_golden = false;
};
}}
#endif
