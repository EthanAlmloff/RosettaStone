#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class StartCombatHandStatsTask { public: explicit StartCombatHandStatsTask(int repeats):m_repeats(repeats){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_repeats;};
}}
