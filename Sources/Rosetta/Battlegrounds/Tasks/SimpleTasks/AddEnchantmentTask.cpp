// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Actions/Generic.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/LifecycleEnchantment.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/IncludeTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
AddEnchantmentTask::AddEnchantmentTask(const std::string_view& cardID,
                                       EntityType entityType, bool useScriptTag)
    : m_cardID(cardID), m_entityType(entityType), m_useScriptTag(useScriptTag)
{
    // Do nothing
}

TaskStatus AddEnchantmentTask::Run(Player& player, Minion& source)
{
    int num = 0;
    if (m_useScriptTag)
    {
        num = player.taskStack.num;
    }

    auto minions = IncludeTask::GetMinions(m_entityType, player, source);
    if (m_cardID == "BG31_812e" || m_cardID == "BG31_812e2")
    {
        for (auto& minion : minions)
            ApplyIchoronLifecycleEnchantment(minion.get(), m_cardID);
        return TaskStatus::COMPLETE;
    }
    Card enchantmentCard = Cards::FindCardByID(m_cardID);

    for (auto& minion : minions)
    {
        if (ApplyReviewedTemporaryChildEnchantment(minion.get(), m_cardID,
                                                   num, num))
            continue;
        // reference_wrapper owns the same target as the historical
        // ApplyReviewedPersistentChildEnchantment(*minion, m_cardID, num)
        // call shape; .get() is the type-correct spelling here.
        if (ApplyReviewedPersistentChildEnchantment(minion.get(), m_cardID, num))
            continue;
        Generic::AddEnchantment(enchantmentCard, minion, num);
    }

    return TaskStatus::COMPLETE;
}

TaskStatus AddEnchantmentTask::Run(Player& player, Minion& source,
                                   Minion& target)
{
    auto minions =
        IncludeTask::GetMinions(m_entityType, player, source, target);
    if (m_cardID == "BG31_812e" || m_cardID == "BG31_812e2")
    {
        for (auto& minion : minions)
            ApplyIchoronLifecycleEnchantment(minion.get(), m_cardID);
        return TaskStatus::COMPLETE;
    }
    Card enchantmentCard = Cards::FindCardByID(m_cardID);

    for (auto& minion : minions)
    {
        if (ApplyReviewedTemporaryChildEnchantment(minion.get(), m_cardID))
            continue;
        // reference_wrapper owns the same target as the historical
        // ApplyReviewedPersistentChildEnchantment(*minion, m_cardID) call
        // shape; .get() is the type-correct spelling here.
        if (ApplyReviewedPersistentChildEnchantment(minion.get(), m_cardID))
            continue;
        Generic::AddEnchantment(enchantmentCard, minion);
    }

    return TaskStatus::COMPLETE;
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
