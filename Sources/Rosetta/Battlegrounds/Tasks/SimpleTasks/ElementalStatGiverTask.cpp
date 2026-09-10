#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ElementalStatGiverTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ElementalStatGiverTask::Run(Player& p, Minion& target) {
    if (!target.HasRace(Race::ELEMENTAL)) return TaskStatus::STOP;
    int attack = m_attack;
    int health = m_health;
    for (const auto& trinket : p.season14.trinkets) {
        if (!trinket.active || trinket.remainingUses == 0) continue;
        const auto behavior = FindTrinketBehavior(
            Cards::FindCardByDbfID(trinket.dbfID).id);
        if (behavior.effect == TrinketEffect::ELEMENTAL_STAT_GIVER_BONUS) {
            attack += behavior.attack;
            health += behavior.health;
        } else if (behavior.effect ==
                   TrinketEffect::ESCALATING_ELEMENTAL_STAT_GIVER_BONUS) {
            attack += behavior.attack + trinket.statScale;
            health += behavior.health + trinket.statScale;
        }
    }
    target.SetAttack(target.GetAttack() + attack);
    target.SetHealth(target.GetHealth() + health);
    return TaskStatus::COMPLETE;
}

TaskStatus ElementalStatGiverTask::Run(Player& p, Minion&, Minion& target) {
    return Run(p, target);
}
}
