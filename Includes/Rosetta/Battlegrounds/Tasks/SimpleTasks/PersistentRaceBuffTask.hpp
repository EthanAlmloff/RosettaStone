#ifndef ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_RACE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class PersistentRaceBuffTask { public:
    PersistentRaceBuffTask(Race race, int attack, int health) : m_race(race), m_attack(attack), m_health(health) {}
    TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
    Race GetRace() const noexcept { return m_race; }
private: Race m_race; int m_attack; int m_health;
}; }}
#endif
