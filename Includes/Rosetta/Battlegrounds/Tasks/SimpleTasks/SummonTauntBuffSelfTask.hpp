#ifndef ROSETTASTONE_BATTLEGROUNDS_SUMMON_TAUNT_BUFF_SELF_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_SUMMON_TAUNT_BUFF_SELF_TASK_HPP

#include <Rosetta/Common/Enums/TaskEnums.hpp>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;
namespace SimpleTasks
{
class SummonTauntBuffSelfTask
{
 public:
    SummonTauntBuffSelfTask(int attack, int health)
        : m_attack(attack), m_health(health) {}
    TaskStatus Run(Player&, Minion& owner, Minion& summoned);
    TaskStatus Run(Player&, Minion& owner);
    int GetAttack() const { return m_attack; }
    int GetHealth() const { return m_health; }
 private:
    int m_attack;
    int m_health;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif
