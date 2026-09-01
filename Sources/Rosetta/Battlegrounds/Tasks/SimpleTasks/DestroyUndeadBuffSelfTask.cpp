#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DestroyUndeadBuffSelfTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus DestroyUndeadBuffSelfTask::Run(Player& player, Minion& source) {
 if ((m_attack<=0 && m_health<=0) || source.IsDestroyed()) return TaskStatus::STOP;
 TaskStatus result = TaskStatus::STOP;
 player.recruitField.ForEachAlive([&](MinionData& data) {
  if (result != TaskStatus::STOP) return;
  auto& target = data.value();
  if (&target != &source && target.HasRace(Race::UNDEAD)) {
   target.SetReborn(true); target.TakeDamage(target.GetHealth());
   source.SetAttack(source.GetAttack() + m_attack);
   source.SetHealth(source.GetHealth() + m_health);
   result = TaskStatus::COMPLETE;
  }
 });
 return result;
}
TaskStatus DestroyUndeadBuffSelfTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
