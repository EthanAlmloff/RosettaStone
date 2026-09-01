#pragma once

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds {
class Minion;
class Player;
namespace SimpleTasks {
class StartCombatHighestHandMinionSummonTask {
public:
    StartCombatHighestHandMinionSummonTask(int attack, int health)
        : m_attack(attack), m_health(health) {}
    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);
private:
    int m_attack;
    int m_health;
};
} // namespace SimpleTasks
} // namespace RosettaStone::Battlegrounds
