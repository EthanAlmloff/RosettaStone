#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpendGoldThresholdSpellTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SpendGoldThresholdSpellTask::Run(Player& player, Minion& source) {
  const int spent = player.season14.goldSpentThisTurn;
  if (spent < m_threshold) { source.SetSpendGoldThresholdFired(false); return TaskStatus::COMPLETE; }
  if (source.SpendGoldThresholdFired()) return TaskStatus::COMPLETE;
  if (player.CastTavernSpellFree(m_spellID, m_amount))
    source.SetSpendGoldThresholdFired(true);
  return TaskStatus::COMPLETE;
}
TaskStatus SpendGoldThresholdSpellTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
