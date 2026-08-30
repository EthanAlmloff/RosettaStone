// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Actions/Generic.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
FriendlyRaceEnchantmentTask::FriendlyRaceEnchantmentTask(
    const std::string_view& cardID, Race race, bool excludeSource)
    : m_cardID(cardID), m_race(race), m_excludeSource(excludeSource)
{
    // Do nothing
}

TaskStatus FriendlyRaceEnchantmentTask::Run(
    Player& player, Minion& source)
{
    Card enchantmentCard = Cards::FindCardByID(m_cardID);
    player.GetField().ForEachAlive(
        [this, &enchantmentCard, &source](MinionData& data) {
        Minion& minion = data.value();
        const bool isSource = &minion == &source ||
                              (source.GetPoolIndex() >= 0 &&
                               minion.GetPoolIndex() == source.GetPoolIndex()) ||
                              (source.GetIndex() >= 0 &&
                               minion.GetIndex() == source.GetIndex());
        if (minion.HasRace(m_race) && (!m_excludeSource || !isSource))
        {
            Generic::AddEnchantment(enchantmentCard, minion);
        }
        });

    return TaskStatus::COMPLETE;
}

TaskStatus FriendlyRaceEnchantmentTask::Run(
    Player& player, Minion& source, [[maybe_unused]] Minion& target)
{
    return Run(player, source);
}
}  // namespace RosettaStone::Battlegrounds::SimpleTasks
