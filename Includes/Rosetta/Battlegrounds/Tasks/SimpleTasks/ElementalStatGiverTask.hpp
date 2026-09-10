#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
//! Resolves a targeted Elemental stat-giver (for example Smogger).
//! Trinket augmentation is applied to this payload only, never to every
//! Elemental play.
class ElementalStatGiverTask {
public:
    ElementalStatGiverTask(int attack, int health)
        : m_attack(attack), m_health(health) {}
    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);
private:
    int m_attack;
    int m_health;
};
}}
