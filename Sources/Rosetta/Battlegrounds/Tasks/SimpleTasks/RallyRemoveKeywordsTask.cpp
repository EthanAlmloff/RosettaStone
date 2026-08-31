#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRemoveKeywordsTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyRemoveKeywordsTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
TaskStatus RallyRemoveKeywordsTask::Run(Player&, Minion& source, Minion& target) {
  if (m_keywordBits <= 0 || &source == &target || source.IsDestroyed() ||
      target.IsDestroyed()) return TaskStatus::STOP;
  if ((m_keywordBits & 1) != 0) target.SetReborn(false);
  if ((m_keywordBits & 2) != 0) target.SetGameTag(GameTag::TAUNT, 0);
  return TaskStatus::COMPLETE;
}
}
