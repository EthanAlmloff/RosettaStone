#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion& source) {
    // Plaguerunner Portrait's account-wide reward is deliberately resolved
    // at the same outside-combat boundary as its generated Deathrattle.  A
    // combat copy must never create a recruit-hand card.
    if (!player.isInCombat &&
        (source.GetCardID() == "BG34_690" ||
         source.GetCardID() == "BG34_690_G")) {
        bool hasPortrait = false;
        for (const auto& trinket : player.season14.trinkets) {
            if (!trinket.active || trinket.remainingUses == 0) continue;
            if (Cards::FindCardByDbfID(trinket.dbfID).id ==
                "BG36_MagicItem_204") {
                hasPortrait = true;
                break;
            }
        }
        if (hasPortrait && !player.hand.IsFull()) {
            const auto copy = Cards::FindCardByID("BG34_690");
            if (copy.dbfID != 0 && copy.hasBehavior)
                player.hand.Add(CardData{Minion(copy)});
        }
    }
    const int attack = m_outsideCombatAttack >= 0 && !player.isInCombat
                           ? m_outsideCombatAttack : m_attack;
    if (!m_excludeSource) {
        player.ApplyPersistentRaceStats(m_race, attack, m_health);
        return TaskStatus::COMPLETE;
    }
    // Apply the same player-wide persistence while withholding the source
    // instance. Future instances still receive the stored aura.
    player.season14.AddPersistentRaceStats(m_race, attack, m_health);
    player.recruitField.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &source) data.value().ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    player.tavern.fieldZone.ForEachAlive([&](MinionData& data) {
        if (&data.value() != &source) data.value().ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    player.hand.ForEach([&](std::optional<CardData>& data) {
        if (data.has_value() && std::holds_alternative<Minion>(data.value()) &&
            &std::get<Minion>(data.value()) != &source)
            std::get<Minion>(data.value()).ApplyPersistentRaceStats(m_race, attack, m_health);
    });
    return TaskStatus::COMPLETE;
}
TaskStatus PersistentRaceBuffTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
