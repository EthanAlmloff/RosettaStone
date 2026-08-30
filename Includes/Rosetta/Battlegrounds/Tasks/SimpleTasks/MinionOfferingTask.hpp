#ifndef ROSETTASTONE_BATTLEGROUNDS_MINION_OFFERING_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MINION_OFFERING_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Creates a public, seeded Discover-style offering of eligible minions.
//! Random/transform followups remain outside this primitive.
class MinionOfferingTask {
 public:
  MinionOfferingTask(Race race, int minTier, int maxTier, int count,
                     bool requiresFriendlyRace = false)
      : m_race(race), m_minTier(minTier), m_maxTier(maxTier), m_count(count),
        m_requiresFriendlyRace(requiresFriendlyRace) {}
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
  Race GetRace() const noexcept { return m_race; }
  int GetMinTier() const noexcept { return m_minTier; }
  int GetMaxTier() const noexcept { return m_maxTier; }
  int GetCount() const noexcept { return m_count; }
  bool RequiresFriendlyRace() const noexcept { return m_requiresFriendlyRace; }
 private:
  Race m_race = Race::INVALID;
  int m_minTier = 1;
  int m_maxTier = 7;
  int m_count = 3;
  bool m_requiresFriendlyRace = false;
};
}}
#endif
