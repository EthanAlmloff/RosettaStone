#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCountRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SpellCountRaceBuffTask::Run(Player& player, Minion& source) {
  const int improvements = m_threshold > 0 ? player.season14.SuccessfulSpellCount() / m_threshold : 0;
  const int attack = m_attack * (1 + improvements);
  const int health = m_health * (1 + improvements);
  if (m_selfBuff)
    source.ApplyPersistentMinionStats(attack, health);
  else
    player.ApplyPersistentRaceStats(m_race, attack, health);
  if (m_alsoLeft && (source.GetCardID() == "BG31_035" ||
                     source.GetCardID() == "BG31_035_G")) {
    bool portrait = false;
    for (const auto& trinket : player.season14.trinkets) {
      if (!trinket.active || trinket.remainingUses == 0) continue;
      const auto behavior = FindTrinketBehavior(
          Cards::FindCardByDbfID(trinket.dbfID).id);
      if (behavior.portraitEffect == PortraitEffect::GROUNDBREAKER_ADJACENT_STATS) {
        portrait = true;
        break;
      }
    }
    if (portrait) {
      const int left = source.GetZonePosition() - 1;
      if (left >= 0 && left < player.recruitField.GetCount() &&
          !player.recruitField[static_cast<std::size_t>(left)].IsDestroyed())
        player.recruitField[static_cast<std::size_t>(left)].ApplyPersistentMinionStats(attack, health);
    }
  }
  return TaskStatus::COMPLETE;
}
TaskStatus SpellCountRaceBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
