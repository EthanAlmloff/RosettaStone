#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <algorithm>
#include <utility>
namespace RosettaStone::Battlegrounds::SimpleTasks {
AddCardTask::AddCardTask(std::string cardID,int amount):m_cardID(std::move(cardID)),m_amount(std::max(0,amount)){}
TaskStatus AddCardTask::Run(Player& player,Minion&) { if(m_amount<=0||player.hand.IsFull()) return TaskStatus::COMPLETE; const auto card=Cards::FindCardByID(m_cardID); if(card.id.empty()||(card.GetCardType()!=CardType::SPELL&&card.GetCardType()!=CardType::BATTLEGROUND_SPELL)) return TaskStatus::COMPLETE; for(int i=0;i<m_amount&&!player.hand.IsFull();++i) player.hand.Add(CardData{Spell(card)}); return TaskStatus::COMPLETE; }
TaskStatus AddCardTask::Run(Player& player,Minion& source,Minion&) { return Run(player,source); }
}
