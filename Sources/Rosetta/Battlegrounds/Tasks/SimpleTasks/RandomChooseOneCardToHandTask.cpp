#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomChooseOneCardToHandTask::Run(Player& player, Minion&) {
 if(m_amount<=0||player.hand.IsFull()) return TaskStatus::STOP; std::vector<const Card*> c;
 for(const auto& card:Cards::GetAllCards()) if(card.isBattlegroundsPoolMinion&&card.normalDbfID==0&&card.hasBehavior&&card.GetCardType()==CardType::MINION&&card.gameTags.contains(GameTag::CHOOSE_ONE)&&card.gameTags.at(GameTag::CHOOSE_ONE)!=0)c.push_back(&card);
 if(c.empty()) return TaskStatus::STOP; for(int i=0;i<m_amount&&!player.hand.IsFull();++i){ Minion generated{*c[Random::get<std::size_t>(0,c.size()-1)]}; player.ApplyFreshMinionModifiers(generated); player.hand.Add(CardData{std::move(generated)}); } return TaskStatus::COMPLETE;
}
TaskStatus RandomChooseOneCardToHandTask::Run(Player& p,Minion& s,Minion&){return Run(p,s);}
}
