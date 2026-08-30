#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BloodGemRaceBonusTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BloodGemRaceBonusTask::Run(Player& player, Minion&) {
  player.season14.AddBloodGemRaceBonus(m_race, m_attack, m_health);
  return TaskStatus::COMPLETE;
}
TaskStatus BloodGemRaceBonusTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
