#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MechDiscoverMagnetizeTask.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MechDiscoverMagnetizeTask::Run(Player& p, Minion& source, Minion& target) {
 if(m_count<=0 || target.IsDestroyed() || !target.HasRace(Race::MECHANICAL)) return TaskStatus::STOP;
 std::vector<Card> candidates; for(const auto& c:Cards::GetAllCards()) if(c.isBattlegroundsPoolMinion&&c.normalDbfID==0&&c.hasBehavior&&c.GetCardType()==CardType::MINION&&c.HasRace(Race::MECHANICAL)&&c.gameTags.contains(GameTag::MAGNETIC)&&c.gameTags.at(GameTag::MAGNETIC)!=0) candidates.push_back(c);
 if(candidates.empty()) return TaskStatus::STOP; effolkronium::random_thread_local::shuffle(candidates.begin(),candidates.end());
 std::vector<Season14Offering> offers; for(std::size_t i=0;i<std::min<std::size_t>(3,candidates.size());++i) offers.push_back({candidates[i].dbfID,0});
 p.season14.pendingMechMagnetizeSourceEntityID=static_cast<std::uint64_t>(source.GetIndex()); p.season14.pendingMechMagnetizeTargetEntityID=static_cast<std::uint64_t>(target.GetIndex()); p.season14.pendingMechMagnetizeRemaining=m_count; p.season14.BeginOfferingDecision(Season14Decision::DISCOVER,static_cast<std::uint64_t>(source.GetIndex()),source.GetDbfID(),std::move(offers)); return TaskStatus::COMPLETE;
}
TaskStatus MechDiscoverMagnetizeTask::Run(Player&,Minion&) { return TaskStatus::STOP; }
}
