#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BuyMinionTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BuyMinionTask::Run(Player&, Minion& source) {
  source.SetAttack(source.GetAttack() + m_attack);
  source.SetHealth(source.GetHealth() + m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus BuyMinionTask::Run(Player&, Minion& source, Minion& target) {
  if (!source.CanUseBuyTrigger(m_maxUses)) return TaskStatus::COMPLETE;
  if (m_maxUses > 0) source.ConsumeBuyTrigger();
  if (m_targetStats) {
    target.SetAttack(target.GetAttack() * m_multiplier + m_attack);
    target.SetHealth(target.GetHealth() * m_multiplier + m_health);
  } else {
    source.SetAttack(source.GetAttack() + (m_multiplier ? target.GetAttack() * m_multiplier : m_attack));
    source.SetHealth(source.GetHealth() + (m_multiplier ? target.GetHealth() * m_multiplier : m_health));
  }
  return TaskStatus::COMPLETE;
}
}
