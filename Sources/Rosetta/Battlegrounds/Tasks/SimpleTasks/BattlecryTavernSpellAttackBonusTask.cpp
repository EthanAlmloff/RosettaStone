#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellAttackBonusTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BattlecryTavernSpellAttackBonusTask::Run(Player& player, Minion&) {
  player.season14.tavernSpellAttackBonus += m_attack;
  return TaskStatus::COMPLETE;
}
TaskStatus BattlecryTavernSpellAttackBonusTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
