// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomGoldenTierMinionToHandTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RandomGoldenTierMinionToHandTask::Run(Player& p, Minion&) {
 if(m_amount<=0 || p.hand.IsFull()) return TaskStatus::STOP;
 std::vector<const Card*> candidates;
 for(const auto& card: Cards::GetAllCards())
   if(card.isBattlegroundsPoolMinion && card.GetCardType()==CardType::MINION &&
      card.normalDbfID==0 && card.premiumDbfID!=0 && card.GetTier()==4) candidates.push_back(&card);
 if(candidates.empty()) return TaskStatus::STOP;
 for(int i=0;i<m_amount && !p.hand.IsFull() && !candidates.empty();++i){
   const auto index=Random::get<std::size_t>(0,candidates.size()-1);
   const Card base = *candidates[index];
   const Card golden = Cards::FindCardByDbfID(base.premiumDbfID);
   if (golden.dbfID == 0 || golden.GetCardType() != CardType::MINION) continue;
   Minion generated{golden};
   p.ApplyFreshMinionModifiers(generated);
   p.hand.Add(CardData{std::move(generated)});
   // A single Battlecry resolves distinct pool picks; do not manufacture a
   // triple by selecting the same source again for the golden count.
   candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(index));
 }
 return TaskStatus::COMPLETE;
}
TaskStatus RandomGoldenTierMinionToHandTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
