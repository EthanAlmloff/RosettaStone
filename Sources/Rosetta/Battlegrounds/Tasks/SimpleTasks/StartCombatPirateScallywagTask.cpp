#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatPirateScallywagTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus StartCombatPirateScallywagTask::Run(Player& player, Minion& source) {
  const auto card = m_golden ? std::string_view{"TB_BaconUps_141"} : std::string_view{"BGS_061"};
  // Start-of-combat tasks run after the recruit field has been copied into
  // battleField.  Resolve against GetField() so the payload is attached to
  // the actual combat pirates (and not to the stale recruit snapshots).
  player.GetField().ForEachAlive([&](MinionData& data) {
    auto& pirate = data.value();
    if (!pirate.IsSameInstance(source) && pirate.HasRace(Race::PIRATE)) {
      for (int i = 0; i < m_repeats; ++i)
        pirate.AddDarkGiftDeathrattleTask(TaskType{SummonTask{card, 1, SummonSide::DEATHRATTLE}});
    }
  });
  return TaskStatus::COMPLETE;
}
TaskStatus StartCombatPirateScallywagTask::Run(Player& player, Minion& source, Minion&) {
  return Run(player, source);
}
}
