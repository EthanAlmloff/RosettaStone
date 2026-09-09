// Copyright (c) 2026 Hearthstone BG AI contributors

#include <Rosetta/Battlegrounds/Actions/Generic.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/LifecycleEnchantment.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <effolkronium/random.hpp>

#include <vector>

namespace RosettaStone::Battlegrounds::SimpleTasks
{
namespace
{
bool IsLeapfroggerChild(std::string_view cardID)
{
    return cardID == "BG21_000e" || cardID == "BG21_000_Ge";
}

bool IsGoldrinnChild(std::string_view cardID)
{
    return cardID == "BGS_018e";
}
}

FriendlyRaceEnchantmentTask::FriendlyRaceEnchantmentTask(
    const std::string_view& cardID, Race race, bool excludeSource)
    : m_cardID(cardID), m_race(race), m_excludeSource(excludeSource)
{
    // Do nothing
}

TaskStatus FriendlyRaceEnchantmentTask::Run(
    Player& player, Minion& source)
{
    // Leapfrogger's child is a single random friendly Beast, and is only a
    // combat deathrattle.  Keep this special case here instead of weakening
    // the ordinary race-wide task used by unrelated cards.
    if (IsLeapfroggerChild(m_cardID))
    {
        if (!player.isInCombat) return TaskStatus::STOP;
        std::vector<Minion*> candidates;
        player.GetField().ForEachAlive(
            [this, &source, &candidates](MinionData& data) {
                Minion& minion = data.value();
                if (&minion != &source && minion.HasRace(m_race))
                    candidates.push_back(&minion);
            });
        if (candidates.empty()) return TaskStatus::STOP;
        const auto index = effolkronium::random_thread_local::get<std::size_t>(
            0, candidates.size() - 1);
        return ApplyReviewedPersistentChildEnchantment(
                   *candidates[index], m_cardID)
                   ? TaskStatus::COMPLETE
                   : TaskStatus::STOP;
    }

    // Goldrinn's normal and golden parents both use the canonical Soul of
    // the Beast child.  The golden parent invokes this task twice; route
    // each application through the typed lifecycle gate so the +8/+8 payload
    // and next-turn expiry are retained without losing child provenance.
    if (IsGoldrinnChild(m_cardID))
    {
        bool applied = false;
        player.GetField().ForEachAlive(
            [this, &source, &applied](MinionData& data) {
                Minion& minion = data.value();
                const bool isSource = &minion == &source ||
                                      (source.GetPoolIndex() >= 0 &&
                                       minion.GetPoolIndex() == source.GetPoolIndex()) ||
                                      (source.GetIndex() >= 0 &&
                                       minion.GetIndex() == source.GetIndex());
                if (minion.HasRace(m_race) &&
                    (!m_excludeSource || !isSource))
                {
                    const bool resolved = ApplyReviewedLifecycleEnchantment(
                        minion, "BGS_018", m_cardID,
                        Minion::TemporaryEnchantment::Stats, 8, 8);
                    if (resolved) {
                        minion.RecordTemporaryEnchantmentOccurrence(m_cardID);
                        applied = true;
                    }
                }
            });
        return applied ? TaskStatus::COMPLETE : TaskStatus::STOP;
    }

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
            if (!ApplyReviewedPersistentChildEnchantment(minion, m_cardID))
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
