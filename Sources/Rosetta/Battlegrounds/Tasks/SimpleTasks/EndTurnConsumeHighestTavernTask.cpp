// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnConsumeHighestTavernTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnConsumeHighestTavernTask::Run(Player& p, Minion& s) {
 if (m_multiplier<=0 || s.IsDestroyed()) return TaskStatus::STOP;
 int best=-1, health=-1;
 // The Tavern may also contain spell slots, but those are deliberately not
 // part of fieldZone and can never be consumed by this minion task.  The
 // strict comparison makes the left-most highest-health minion the stable
 // tie-breaker (ForEachAlive follows zone order).
 p.tavern.fieldZone.ForEachAlive([&](MinionData& d){
   if(d.value().GetPoolIndex()>=0 && d.value().GetHealth()>health){
     health=d.value().GetHealth();
     best=d.value().GetZonePosition();
   }
 });
 if(best<0) return TaskStatus::STOP;
 Minion consumed=p.tavern.fieldZone.Remove(p.tavern.fieldZone[static_cast<std::size_t>(best)]);
 if (consumed.GetPoolIndex() >= 0)
     p.returnMinionCallback(consumed.GetPoolIndex());
 s.SetAttack(s.GetAttack()+consumed.GetAttack()*m_multiplier);
 s.SetHealth(s.GetHealth()+consumed.GetHealth()*m_multiplier);
 return TaskStatus::COMPLETE;
}
TaskStatus EndTurnConsumeHighestTavernTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
