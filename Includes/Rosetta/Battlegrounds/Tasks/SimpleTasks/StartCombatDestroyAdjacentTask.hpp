#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class StartCombatDestroyAdjacentTask { public: explicit StartCombatDestroyAdjacentTask(bool both):m_both(both){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: bool m_both; };
}}
