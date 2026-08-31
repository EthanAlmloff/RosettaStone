#ifndef ROSETTASTONE_BATTLEGROUNDS_RALLY_RANDOM_RACE_KEYWORD_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RALLY_RANDOM_RACE_KEYWORD_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class RallyRandomRaceKeywordTask { public:
  RallyRandomRaceKeywordTask(Race race, GameTag tag, int amount) : m_race(race), m_tag(tag), m_amount(amount) {}
  TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: Race m_race; GameTag m_tag; int m_amount;
};
} }
#endif
