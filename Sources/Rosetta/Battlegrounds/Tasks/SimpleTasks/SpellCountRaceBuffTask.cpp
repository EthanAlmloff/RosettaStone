#include <Rosetta/Battlegrounds/Models/Player.hpp>
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
  return TaskStatus::COMPLETE;
}
TaskStatus SpellCountRaceBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
