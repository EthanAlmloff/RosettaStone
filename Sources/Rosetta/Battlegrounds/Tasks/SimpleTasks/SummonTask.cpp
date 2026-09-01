// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <stdexcept>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
SummonTask::SummonTask(const std::string_view& cardID, int amount,
                       SummonSide side, bool addToStack)
    : m_cardID(cardID), m_side(side), m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

int SummonTask::GetPosition(Minion& source, SummonSide side)
{
    int summonPos;

    switch (side)
    {
        case SummonSide::DEFAULT:
        {
            summonPos = -1;
            break;
        }
        case SummonSide::LEFT:
        {
            if (source.GetZoneType() == ZoneType::PLAY)
            {
                summonPos = source.GetZonePosition();
            }
            else
            {
                summonPos = source.GetLastFieldPos();
            }
            break;
        }
        case SummonSide::RIGHT:
        {
            if (source.GetZoneType() == ZoneType::PLAY)
            {
                summonPos = source.GetZonePosition() + 1;
            }
            else
            {
                summonPos = source.GetLastFieldPos();
            }
            break;
        }
        case SummonSide::DEATHRATTLE:
        {
            summonPos = source.GetLastFieldPos();
            break;
        }
        default:
            throw std::invalid_argument(
                "SummonTask::Impl() - Invalid summon side");
    }

    return summonPos;
}

TaskStatus SummonTask::Run(Player& player, Minion& source)
{
    const Card card = Cards::FindCardByID(m_cardID);

    if (m_amount <= 0 || card.id.empty())
    {
        return m_amount <= 0 ? TaskStatus::COMPLETE : TaskStatus::STOP;
    }

    // Compute the source-relative insertion point once.  Inserting to the
    // left/right can shift the source inside FieldZone, invalidating its old
    // zone position before a multi-summon task reaches its next iteration.
    const int sourceSummonPos = GetPosition(source, m_side);

    for (int i = 0; i < m_amount; ++i)
    {
        if (player.GetField().IsFull())
        {
            return TaskStatus::STOP;
        }

        Minion summonMinion{ card };
        // Passive hero-power auras apply to every fresh owned minion, not
        // merely to entities that originated in the Tavern.
        player.ApplyFreshMinionModifiers(summonMinion);
        summonMinion.getPlayerCallback = [&player]() -> Player& {
            return player;
        };
        if (player.getNextCardIndexCallback)
        {
            summonMinion.SetIndex(player.getNextCardIndexCallback());
        }

        int summonPos = sourceSummonPos;
        if (summonPos > player.GetField().GetCount())
        {
            summonPos = player.GetField().GetCount();
        }
        if (summonPos < 0)
        {
            summonPos = player.GetField().GetCount();
        }

        player.GetField().Add(summonMinion, summonPos);
        const int addedPos = summonPos < player.GetField().GetCount()
                                 ? summonPos
                                 : player.GetField().GetCount() - 1;
        Minion& summoned = player.GetField()[addedPos];
        player.ApplyTamuzoCombatSummon(summoned);

        player.GetField().ForEachAlive([&summoned](MinionData& aliveMinion) {
            aliveMinion.value().ActivateTrigger(TriggerType::SUMMON,
                                                summoned);
        });
        player.ApplySummonTrinkets(summoned);

        if (m_addToStack)
        {
            player.taskStack.minions.emplace_back(summoned);
        }
    }

    return TaskStatus::COMPLETE;
}

TaskStatus SummonTask::Run(Player& player, Minion& source,
                           [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
