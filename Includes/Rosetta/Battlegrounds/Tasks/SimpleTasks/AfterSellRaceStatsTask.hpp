#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <Rosetta/Common/Enums/CardEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;

namespace SimpleTasks {
//! Gives the observing minion the sold entity's current stats when the sold
//! entity has the requested race.  The sold entity is passed as the trigger
//! source, including after it has been removed from the recruit field.
class AfterSellRaceStatsTask {
 public:
  AfterSellRaceStatsTask(Race race, int multiplier)
      : m_race(race), m_multiplier(multiplier) {}
  TaskStatus Run(Player&, Minion& owner, Minion& sold);
  TaskStatus Run(Player&, Minion& owner);
  Race RaceFilter() const noexcept { return m_race; }
  int Multiplier() const noexcept { return m_multiplier; }

 private:
  Race m_race;
  int m_multiplier;
};
}
}
