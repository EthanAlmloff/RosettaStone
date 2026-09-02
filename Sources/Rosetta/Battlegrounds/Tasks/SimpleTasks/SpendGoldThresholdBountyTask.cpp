#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpendGoldThresholdBountyTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus SpendGoldThresholdBountyTask::Run(Player& player, Minion& source) {
  const int spent = player.season14.goldSpentThisTurn;
  if (spent < m_threshold) {
    source.SetSpendGoldThresholdFired(false);
    source.SetSpendGoldThresholdCount(0);
    return TaskStatus::COMPLETE;
  }
  const int reached = spent / m_threshold;
  int delivered = source.SpendGoldThresholdCount();
  while (delivered < reached) {
    // Deliver one Bounty at a time so a nearly-full hand can resume the
    // same threshold without losing or duplicating the remainder.
    if (player.hand.GetCount() + m_amount > MAX_HAND_SIZE)
      return TaskStatus::STOP;
    const auto status = RandomBountyToHandTask{m_amount}.Run(player, source);
    if (status != TaskStatus::COMPLETE) return status;
    ++delivered;
    source.SetSpendGoldThresholdCount(delivered);
  }
  source.SetSpendGoldThresholdFired(delivered > 0);
  return TaskStatus::COMPLETE;
}
TaskStatus SpendGoldThresholdBountyTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
