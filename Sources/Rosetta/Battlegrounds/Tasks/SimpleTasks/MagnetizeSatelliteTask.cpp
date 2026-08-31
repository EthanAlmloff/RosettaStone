#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizeSatelliteTask.hpp>
#include <utility>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus MagnetizeSatelliteTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
TaskStatus MagnetizeSatelliteTask::Run(Player&, Minion&, Minion& target) {
 if(!target.HasRace(Race::MECHANICAL)||target.IsDestroyed()) return TaskStatus::STOP;
 const auto card=Cards::FindCardByID("BG31_171te"); if(card.id.empty()) return TaskStatus::STOP;
 for(int i=0;i<m_repeats;++i){ Minion satellite{card}; satellite.SetAttack(m_attack); satellite.SetHealth(m_health); satellite.MagnetizeOnto(target); m_attack+=m_increment; m_health+=m_increment; }
 return TaskStatus::COMPLETE;
}
}
