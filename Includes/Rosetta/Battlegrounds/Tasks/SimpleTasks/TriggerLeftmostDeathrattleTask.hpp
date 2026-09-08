#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks { class TriggerLeftmostDeathrattleTask { public: explicit TriggerLeftmostDeathrattleTask(int repeats = 1):m_repeats(repeats){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); int Repeats() const noexcept { return m_repeats; } private: int m_repeats; }; }}
