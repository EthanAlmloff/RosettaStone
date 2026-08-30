#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRaceBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyRaceBuffTask::Run(Player& player, Minion&) {
  player.GetField().ForEachAlive([this](MinionData& data) {
    auto& minion = data.value();
    // Race::ALL is a selector wildcard, not a literal minion race.
    if (m_race == Race::ALL || minion.HasRace(m_race)) {
      minion.SetAttack(minion.GetAttack() + m_attack);
      minion.SetHealth(minion.GetHealth() + m_health);
    }
  });
  return TaskStatus::COMPLETE;
}
TaskStatus RallyRaceBuffTask::Run(Player& player, Minion& source, Minion&) {
  if (m_triggerRace != Race::INVALID && !source.HasRace(m_triggerRace)) {
    return TaskStatus::COMPLETE;
  }
  return Run(player, source);
}
}
