// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Models/Battle.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/IncludeTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
namespace
{
// AttackTask is also used by immediate/forced attacks.  Keep the combat
// state scoped even when a task callback aborts the exchange, so an
// Invulnerability gift cannot leak into a later non-attack damage event.
struct AttackingStateGuard
{
    Minion& minion;
    const bool previous;

    explicit AttackingStateGuard(Minion& value)
        : minion(value), previous(value.IsAttacking())
    {
        minion.SetAttacking(true);
    }

    ~AttackingStateGuard() { minion.SetAttacking(previous); }
};
}  // namespace

AttackTask::AttackTask(EntityType attacker) : m_attacker(attacker)
{
    // Do nothing
}

TaskStatus AttackTask::Run(Player& player, Minion& source)
{
    Battle& battle = player.getBattleCallback();

    auto attackers = IncludeTask::GetMinions(m_attacker, player, source);
    for (auto& attacker : attackers)
    {
        Minion& battleTarget = battle.GetProperTarget(attacker);
        // Forced/immediate attacks still count as attacks for Dark Gifts such
        // as Invulnerability.  Preserve any enclosing attack state, since a
        // Rally can legally dispatch another AttackTask during combat.
        AttackingStateGuard attacking(attacker.get());
        battleTarget.TakeDamage(attacker);
        attacker.get().TakeDamage(battleTarget);

        battle.ProcessDestroy(false);
    }

    return TaskStatus::COMPLETE;
}

TaskStatus AttackTask::Run(Player& player, Minion& source, Minion& target)
{
    Battle& battle = player.getBattleCallback();

    auto attackers =
        IncludeTask::GetMinions(m_attacker, player, source, target);
    for (auto& attacker : attackers)
    {
        Minion& battleTarget = battle.GetProperTarget(attacker);
        AttackingStateGuard attacking(attacker.get());
        battleTarget.TakeDamage(attacker);
        attacker.get().TakeDamage(battleTarget);

        battle.ProcessDestroy(false);
    }

    return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
