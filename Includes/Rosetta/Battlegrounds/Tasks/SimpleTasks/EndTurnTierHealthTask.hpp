#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class EndTurnTierHealthTask { public: explicit EndTurnTierHealthTask(int multiplier=1):m_multiplier(multiplier){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: int m_multiplier; };
}}
