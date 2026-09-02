#ifndef ROSETTASTONE_BATTLEGROUNDS_START_COMBAT_SPELL_SCALED_RACE_BUFF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_START_COMBAT_SPELL_SCALED_RACE_BUFF_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Applies the Start of Combat race grant, improved once per Tavern spell.
class StartCombatSpellScaledRaceBuffTask
{
 public:
    StartCombatSpellScaledRaceBuffTask(Race race, int attack, int health,
                                       int improvementPerSpell)
        : m_race(race), m_attack(attack), m_health(health),
          m_improvementPerSpell(improvementPerSpell)
    {
    }

    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);

    int Attack() const noexcept { return m_attack; }
    int Health() const noexcept { return m_health; }
    int ImprovementPerSpell() const noexcept { return m_improvementPerSpell; }

 private:
    Race m_race;
    int m_attack = 0;
    int m_health = 0;
    int m_improvementPerSpell = 0;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
