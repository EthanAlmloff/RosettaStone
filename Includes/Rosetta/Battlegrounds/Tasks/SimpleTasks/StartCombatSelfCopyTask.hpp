#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class StartCombatSelfCopyTask { public: explicit StartCombatSelfCopyTask(int amount):m_amount(amount){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: int m_amount; };
}}
