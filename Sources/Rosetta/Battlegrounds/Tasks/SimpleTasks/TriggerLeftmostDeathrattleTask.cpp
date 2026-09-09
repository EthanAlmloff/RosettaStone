#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion& source) {
  // Macaw's text explicitly says "another" minion.  Exclude the observing
  // source even when it has acquired a deathrattle through another effect.
  // Rally is dispatched during combat, where the authoritative friendly
  // entities live in battleField.  GetField() also keeps direct/unit-test
  // invocations on recruitField correct when the player is not in combat.
  auto& field = p.GetField();
  bool triggered = false;
  for (int repeat = 0; repeat < m_repeats; ++repeat) {
    Minion* target = nullptr;
    // Re-select immediately before each repeat.  A deathrattle can mutate or
    // remove its owner (or otherwise change its payload), so retaining a raw
    // target pointer across repeats is unsafe and can bypass leftmost order.
    // The combat-aware field is authoritative here.  The historical source
    // shape was `for (int i = 0; i < p.recruitField.GetCount(); ++i)`;
    // recruitField is intentionally not used during Rally dispatch.
    for (int i = 0; i < field.GetCount(); ++i) {
      auto& m = field[i];
      const bool isSource = &m == &source ||
                            (source.GetIndex() >= 0 &&
                             m.GetIndex() == source.GetIndex());
      if (!m.IsDestroyed() && !isSource && m.HasDeathrattle()) {
        target = &m;
        break;
      }
    }
    if (target == nullptr) break;
    target->ActivateTask(PowerType::DEATHRATTLE, p);
    triggered = true;
  }
  // Macaw Portrait adds a second, independent Rally payload. Keep the
  // ordinary Monstrous Macaw deathrattle trigger above intact, then resolve
  // the left-most other Battlecry from the same combat snapshot.
  if (p.HasActivePortrait(PortraitEffect::MACAW_LEFTMOST_BATTLECRY) &&
      (source.GetCardID() == "BGS_078" ||
       source.GetCardID() == "TB_BaconUps_135")) {
    for (int repeat = 0; repeat < m_repeats; ++repeat) {
      Minion* target = nullptr;
      for (int i = 0; i < field.GetCount(); ++i) {
        auto& candidate = field[i];
        const bool isSource = &candidate == &source ||
                              (source.GetIndex() >= 0 &&
                               candidate.GetIndex() == source.GetIndex());
        if (!candidate.IsDestroyed() && !isSource && candidate.HasBattlecry()) {
          target = &candidate;
          break;
        }
      }
      if (target == nullptr) break;
      target->ActivateTask(PowerType::POWER, p);
      triggered = true;
      if (p.season14.pendingDecision != Season14Decision::NONE) break;
    }
  }
  return triggered ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus TriggerLeftmostDeathrattleTask::Run(Player& p, Minion& s,
                                                Minion&) {
  return Run(p, s);
}
}
