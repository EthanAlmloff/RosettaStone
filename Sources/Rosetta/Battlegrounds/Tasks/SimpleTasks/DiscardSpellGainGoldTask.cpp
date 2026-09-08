#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DiscardSpellGainGoldTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DiscardSpellGainGoldTask::Run(Player& player, Minion&) {
  for (int i = 0; i < player.hand.GetCount(); ++i) {
    auto& card = player.hand[i];
    if (std::holds_alternative<Spell>(card)) {
      player.hand.Remove(card);
      player.OnCardDiscarded();
      player.remainCoin += m_amount;
      return TaskStatus::COMPLETE;
    }
  }
  return TaskStatus::STOP;
}
TaskStatus DiscardSpellGainGoldTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
