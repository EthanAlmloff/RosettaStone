#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateFishbaitTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ActivateFishbaitTask::Run(Player& p, Minion& s) { return RunAt(p, s, 0); }
TaskStatus ActivateFishbaitTask::RunAt(Player& p, Minion& s, std::size_t index) {
  if (s.IsDestroyed() || index >= static_cast<std::size_t>(p.tavern.fieldZone.GetCount())) return TaskStatus::STOP;
  auto bait = Cards::FindCardByID(m_id);
  if (bait.id.empty() || p.tavern.fieldZone[index].IsDestroyed()) return TaskStatus::STOP;
  const bool frozen = p.tavern.fieldZone[index].IsFrozen();
  auto replaced = p.tavern.fieldZone.Remove(p.tavern.fieldZone[index]);
  p.returnMinionCallback(replaced.GetPoolIndex());
  bool attacked = false;
  p.recruitField.ForEachAlive([&](MinionData& d) { if (!attacked && d.value().HasRace(Race::BEAST)) { d.value().SetAttack(d.value().GetAttack() + m_stat); d.value().SetHealth(d.value().GetHealth() + m_stat); attacked = true; } });
  Minion generated{bait};
  generated.SetFrozen(frozen);
  p.tavern.fieldZone.Add(std::move(generated), static_cast<int>(index));
  return TaskStatus::COMPLETE;
}
TaskStatus ActivateFishbaitTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
