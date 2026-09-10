// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnConsumeHighestTavernTask.hpp>
#include <effolkronium/random.hpp>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus EndTurnConsumeHighestTavernTask::Run(Player& p, Minion& s) {
 if (m_multiplier<=0 || s.IsDestroyed()) return TaskStatus::STOP;
 int best=-1, health=-1;
 std::vector<int> ties;
 // The Tavern may also contain spell slots, but those are deliberately not
 // part of fieldZone and can never be consumed by this minion task.  The
 // Equal-health offers are equally eligible; resolve the tie through the
 // simulator RNG rather than leaking Tavern slot order into the effect.
 p.tavern.fieldZone.ForEachAlive([&](MinionData& d){
   if(d.value().GetPoolIndex()>=0 && d.value().GetHealth()>health){
     health=d.value().GetHealth();
     ties.clear();
     ties.push_back(d.value().GetZonePosition());
   } else if (d.value().GetPoolIndex()>=0 && d.value().GetHealth()==health) {
     ties.push_back(d.value().GetZonePosition());
   }
 });
 if (!ties.empty()) best = ties[Random::get<std::size_t>(0, ties.size() - 1)];
 if(best<0) return TaskStatus::STOP;
 Minion consumed=p.tavern.fieldZone.Remove(p.tavern.fieldZone[static_cast<std::size_t>(best)]);
 if (consumed.GetPoolIndex() >= 0)
     p.returnMinionCallback(consumed.GetPoolIndex());
 s.SetAttack(s.GetAttack()+consumed.GetAttack()*m_multiplier);
 s.SetHealth(s.GetHealth()+consumed.GetHealth()*m_multiplier);
 p.ApplyDemonConsumeBonus(s, consumed);
 // This is a real Tavern-minion consume, even though it is resolved by a
 // recruit-end task rather than ConsumeRandomTavernTask.  Keep Statue of
 // Hir'eek (and any future successful-consume observers) on the same
 // post-removal boundary as every other consume path.
 p.ApplyTavernMinionConsumedTrinkets();
 // Flaming Portrait mirrors each successful consume to the live neighbors of
 // the triggering Flaming Enforcer. Resolve this after removing the Tavern
 // card so only the consumed stats are copied and each adjacent instance is
 // updated once, including when the source is golden.
 if (p.HasActivePortrait(PortraitEffect::FLAMING_ENFORCER_ADJACENT_STATS)) {
   const int sourcePosition = s.GetZonePosition();
   const int attack = consumed.GetAttack() * m_multiplier;
   const int health = consumed.GetHealth() * m_multiplier;
   p.GetField().ForEachAlive([sourcePosition, attack, health, &s](MinionData& d) {
     auto& neighbor = d.value();
     if (&neighbor == &s) return;
     const int position = neighbor.GetZonePosition();
     if (position == sourcePosition - 1 || position == sourcePosition + 1)
       neighbor.ApplyPersistentMinionStats(attack, health);
   });
 }
 return TaskStatus::COMPLETE;
}
TaskStatus EndTurnConsumeHighestTavernTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
