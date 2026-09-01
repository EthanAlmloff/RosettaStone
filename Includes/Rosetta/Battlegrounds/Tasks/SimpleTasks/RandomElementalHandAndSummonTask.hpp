#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class RandomElementalHandAndSummonTask { public:
 explicit RandomElementalHandAndSummonTask(int amount):m_amount(amount){}
 int Amount() const { return m_amount; }
 TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_amount; }; }}
