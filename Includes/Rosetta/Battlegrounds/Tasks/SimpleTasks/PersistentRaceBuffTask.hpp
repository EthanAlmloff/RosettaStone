#ifndef ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_PERSISTENT_RACE_BUFF_TASK_HPP
#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks {
class PersistentRaceBuffTask { public:
    PersistentRaceBuffTask(Race race, int attack, int health,
                           bool excludeSource = false,
                           int outsideCombatAttack = -1)
        : m_race(race), m_attack(attack), m_health(health),
          m_excludeSource(excludeSource),
          m_outsideCombatAttack(outsideCombatAttack) {}
    TaskStatus Run(Player&, Minion&); TaskStatus Run(Player&, Minion&, Minion&);
    Race GetRace() const noexcept { return m_race; }
    int GetAttack() const noexcept { return m_attack; }
    int GetHealth() const noexcept { return m_health; }
    bool ExcludesSource() const noexcept { return m_excludeSource; }
    int OutsideCombatAttack() const noexcept { return m_outsideCombatAttack; }
private:
    Race m_race; int m_attack; int m_health;
    bool m_excludeSource = false;
    int m_outsideCombatAttack = -1;
}; }}
#endif
