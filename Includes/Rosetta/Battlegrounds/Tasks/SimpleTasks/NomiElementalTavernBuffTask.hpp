#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class NomiElementalTavernBuffTask { public:
 explicit NomiElementalTavernBuffTask(int amount = 1) : m_amount(amount) {}
 int Amount() const { return m_amount; }
 TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_amount = 1;
}; }}
