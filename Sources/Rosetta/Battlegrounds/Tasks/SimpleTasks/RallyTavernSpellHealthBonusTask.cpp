#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyTavernSpellHealthBonusTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyTavernSpellHealthBonusTask::Run(Player& player, Minion&) {
  player.season14.AddTavernSpellHealthBonus(m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus RallyTavernSpellHealthBonusTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
