#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DistinctSpellRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DistinctSpellRaceBuffTask::Run(Player& p, Minion& source) {
 const int n=static_cast<int>(p.season14.DistinctSpellsThisTurn());
 const int a=m_attack+n*m_scale, h=m_health+n*m_scale;
 p.recruitField.ForEachAlive([&](MinionData& d){if(&d.value()!=&source && d.value().HasRace(Race::NAGA)) d.value().ApplyPersistentMinionStats(a,h);});
 return TaskStatus::COMPLETE;
}
TaskStatus DistinctSpellRaceBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
