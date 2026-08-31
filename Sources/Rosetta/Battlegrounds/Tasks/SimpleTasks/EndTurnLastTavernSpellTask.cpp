// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnLastTavernSpellTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnLastTavernSpellTask::Run(Player& player, Minion&) {
  if (m_amount <= 0 || player.hand.IsFull()) return TaskStatus::STOP;
  const auto card = Cards::FindCardByDbfID(player.season14.LastTavernSpellDbfID());
  if (card.dbfID == 0 || !card.isBattlegroundsPoolSpell) return TaskStatus::STOP;
  for (int i = 0; i < m_amount && !player.hand.IsFull(); ++i)
    player.hand.Add(CardData{Spell(card)});
  return TaskStatus::COMPLETE;
}
TaskStatus EndTurnLastTavernSpellTask::Run(Player& p, Minion& s, Minion&) { return Run(p, s); }
}
