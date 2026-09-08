#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion& source) {
  // Macaw's text explicitly says "another" minion.  Exclude the observing
  // source even when it has acquired a deathrattle through another effect.
  for (int i = 0; i < p.recruitField.GetCount(); ++i) {
    auto& m = p.recruitField[i];
    if (!m.IsDestroyed() &&
        (source.GetIndex() < 0 || m.GetIndex() != source.GetIndex()) &&
        m.HasDeathrattle()) {
      // Resolve the same left-most entity for every repeat.  Encoding a
      // golden Macaw as two independent tasks would re-scan the board after
      // the first trigger and could incorrectly fire a different deathrattle.
      for (int repeat = 0; repeat < m_repeats; ++repeat)
        m.ActivateTask(PowerType::DEATHRATTLE, p);
      return TaskStatus::COMPLETE;
    }
  }
  return TaskStatus::STOP;
}
TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion& s,
                                                Minion&) {
  return Run(p, s);
}
}
