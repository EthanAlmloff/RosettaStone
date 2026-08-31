#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DestroyUndeadBuffSelfTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DestroyUndeadBuffSelfTask::Run(Player& player, Minion& source) {
 if ((m_attack<=0 && m_health<=0) || source.IsDestroyed()) return TaskStatus::STOP;
 for (auto& data : player.recruitField) { auto& target=data.value(); if (&target!=&source && !target.IsDestroyed() && target.HasRace(Race::UNDEAD)) { target.SetReborn(true); target.TakeDamage(target.GetHealth()); source.SetAttack(source.GetAttack()+m_attack); source.SetHealth(source.GetHealth()+m_health); return TaskStatus::COMPLETE; } }
 return TaskStatus::STOP;
}
TaskStatus DestroyUndeadBuffSelfTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
