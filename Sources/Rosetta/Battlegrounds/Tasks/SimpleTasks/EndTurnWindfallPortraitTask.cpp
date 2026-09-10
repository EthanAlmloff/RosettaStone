#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnWindfallPortraitTask.hpp>

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <algorithm>
#include <utility>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnWindfallPortraitTask::Run(Player& player, Minion&) {
  if (m_bonus <= 0 || player.hand.IsFull()) return TaskStatus::COMPLETE;
  const auto card = Cards::FindCardByID("BG31_817");
  if (card.id.empty()) return TaskStatus::STOP;
  Minion tornado(card);
  const auto sold = player.season14.SoldMinionsThisTurn();
  const auto bonus = m_bonus * (1 + std::max(0, sold));
  tornado.SetAttack(tornado.GetAttack() + bonus);
  tornado.SetHealth(tornado.GetHealth() + bonus);
  player.hand.Add(CardData{std::move(tornado)});
  return TaskStatus::COMPLETE;
}

TaskStatus EndTurnWindfallPortraitTask::Run(Player& player, Minion& source,
                                            Minion&) {
  return Run(player, source);
}
}
