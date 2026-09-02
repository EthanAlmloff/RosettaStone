#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConsumeUndeadBattlecryTask.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ConsumeUndeadBattlecryTask::Run(Player& p, Minion& source, Minion& target) {
 if(!target.HasRace(Race::UNDEAD) || target.IsDestroyed() || p.hand.IsFull()) return TaskStatus::STOP;
 if(m_discover){
  std::vector<Card> candidates; for(const auto& c:Cards::GetAllCards()) if(c.isBattlegroundsPoolMinion&&c.normalDbfID==0&&c.HasRace(Race::UNDEAD)) candidates.push_back(c);
  if(candidates.empty()) return TaskStatus::STOP; effolkronium::random_thread_local::shuffle(candidates.begin(),candidates.end());
  p.recruitField.Remove(target);
  std::vector<Season14Offering> offers; for(std::size_t i=0;i<std::min<std::size_t>(3,candidates.size());++i) offers.push_back({candidates[i].dbfID,0});
  p.season14.pendingUndeadDiscoverSourceEntityID=static_cast<std::uint64_t>(source.GetIndex()); p.season14.pendingUndeadDiscoverRemaining=m_copies; p.season14.BeginOfferingDecision(Season14Decision::DISCOVER,static_cast<std::uint64_t>(source.GetIndex()),source.GetDbfID(),std::move(offers)); return TaskStatus::COMPLETE;
 }
 Minion copy=target; p.recruitField.Remove(target); for(int i=0;i<m_copies&&!p.hand.IsFull();++i){ Minion plain{Cards::FindCardByID(copy.GetCardID())}; p.ApplyFreshMinionModifiers(plain); p.hand.Add(CardData{std::move(plain)}); } return TaskStatus::COMPLETE;
}
TaskStatus ConsumeUndeadBattlecryTask::Run(Player&,Minion&) { return TaskStatus::STOP; }
}
