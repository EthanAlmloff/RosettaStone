#ifndef ROSETTASTONE_BATTLEGROUNDS_HIGHEST_HEALTH_ENEMY_DAMAGE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HIGHEST_HEALTH_ENEMY_DAMAGE_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Deals spell-scaled damage to a highest-health enemy minion.
//!
//! Damage is exactly `baseDamage + SuccessfulSpellCount() * damagePerSpell`.
//! `repeats` is one for the normal form and two for the golden form.
//!
//! Ties are resolved through RosettaStone's seeded RNG.  The target is chosen once per task invocation; repeated damage (the golden form) lands on that
//! same target, matching the card's "... twice" wording.
class HighestHealthEnemyDamageTask
{
 public:
    HighestHealthEnemyDamageTask(int baseDamage, int repeats = 1,
                                 int damagePerSpell = 1)
        : m_baseDamage(baseDamage), m_repeats(repeats),
          m_damagePerSpell(damagePerSpell)
    {
    }

    TaskStatus Run(Player&, Minion&);
    TaskStatus Run(Player&, Minion&, Minion&);

    int BaseDamage() const noexcept { return m_baseDamage; }
    int Repeats() const noexcept { return m_repeats; }
    int DamagePerSpell() const noexcept { return m_damagePerSpell; }

 private:
    int m_baseDamage = 0;
    int m_repeats = 1;
    int m_damagePerSpell = 1;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
