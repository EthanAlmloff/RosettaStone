#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/NomiElementalTavernBuffTask.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus NomiElementalTavernBuffTask::Run(Player& p, Minion&) {
  if (m_attack <= 0 && m_health <= 0) return TaskStatus::STOP;
  int attack = m_attack;
  int health = m_health;
  // Fountain Pen augments the payload of an Elemental that grants stats;
  // it does not create an additional play trigger of its own.  Multiple
  // copies stack independently, including normal/golden forms.
  for (const auto& trinket : p.season14.trinkets) {
    if (!trinket.active || trinket.remainingUses == 0) continue;
    const auto behavior = FindTrinketBehavior(
        Cards::FindCardByDbfID(trinket.dbfID).id);
    if (behavior.effect == TrinketEffect::ELEMENTAL_STAT_GIVER_BONUS) {
      attack += behavior.attack;
      health += behavior.health;
    }
  }
  p.ApplyTavernRaceBuff(Race::ELEMENTAL, attack, health);
  return TaskStatus::COMPLETE;
}
TaskStatus NomiElementalTavernBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
