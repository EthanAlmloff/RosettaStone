#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class StartCombatHandSelfCopyTask { public: explicit StartCombatHandSelfCopyTask(bool golden=false):m_golden(golden){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: bool m_golden; };
}}
