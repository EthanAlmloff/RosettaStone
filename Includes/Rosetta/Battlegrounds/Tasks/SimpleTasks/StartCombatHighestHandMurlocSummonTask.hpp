#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class StartCombatHighestHandMurlocSummonTask { public: explicit StartCombatHighestHandMurlocSummonTask(int count):m_count(count){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_count;};
}}
