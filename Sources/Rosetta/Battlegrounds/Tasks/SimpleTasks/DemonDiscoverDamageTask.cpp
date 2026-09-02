#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DemonDiscoverDamageTask.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DemonDiscoverDamageTask::Run(Player& p, Minion& source) {
 if(m_count<=0 || p.hand.IsFull()) return TaskStatus::STOP;
 std::vector<Card> candidates;
 for(const auto& c:Cards::GetAllCards()) if(c.isBattlegroundsPoolMinion && c.normalDbfID==0 && c.HasRace(Race::DEMON)) candidates.push_back(c);
 if(candidates.empty()) return TaskStatus::STOP;
 effolkronium::random_thread_local::shuffle(candidates.begin(),candidates.end());
 std::vector<Season14Offering> offerings;
 for(std::size_t i=0;i<std::min<std::size_t>(3,candidates.size());++i) offerings.push_back({candidates[i].dbfID,0});
 p.season14.pendingDemonDiscoverSourceEntityID=static_cast<std::uint64_t>(source.GetIndex());
 p.season14.pendingDemonDiscoverRemaining=m_count;
 p.season14.BeginOfferingDecision(Season14Decision::DISCOVER,static_cast<std::uint64_t>(source.GetIndex()),source.GetDbfID(),std::move(offerings));
 return TaskStatus::COMPLETE;
}
TaskStatus DemonDiscoverDamageTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
