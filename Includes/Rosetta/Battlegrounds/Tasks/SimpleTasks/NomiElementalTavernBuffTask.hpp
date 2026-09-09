#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class NomiElementalTavernBuffTask { public:
 explicit NomiElementalTavernBuffTask(int amount = 1) : m_attack(amount), m_health(amount) {}
 NomiElementalTavernBuffTask(int attack, int health) : m_attack(attack), m_health(health) {}
 int Attack() const { return m_attack; }
 int Health() const { return m_health; }
 TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
 private: int m_attack = 1;
 int m_health = 1;
}; }}
