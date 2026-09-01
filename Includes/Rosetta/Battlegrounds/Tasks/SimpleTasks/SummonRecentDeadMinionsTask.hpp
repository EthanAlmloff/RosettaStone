#pragma once
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class SummonRecentDeadMinionsTask { public:
 SummonRecentDeadMinionsTask(Race race, int count):m_race(race),m_count(count){}
 Race RaceFilter() const { return m_race; }
 int Count() const { return m_count; }
 TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: Race m_race; int m_count; }; }}
