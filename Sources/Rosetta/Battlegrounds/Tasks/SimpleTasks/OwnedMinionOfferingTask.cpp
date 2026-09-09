#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OwnedMinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Models/Season14.hpp>

namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus OwnedMinionOfferingTask::Run(Player& player, Minion& source,
                                        Minion& target) {
  if (source.GetIndex() < 0 || target.GetIndex() < 0 ||
      target.GetOwnerToken() == nullptr ||
      target.GetOwnerToken() != source.GetOwnerToken() ||
      player.season14.pendingDecision != Season14Decision::NONE ||
      m_sourceCardDbfID == 0)
    return TaskStatus::COMPLETE;
  player.season14.BeginOfferingDecision(
      Season14Decision::DISCOVER,
      static_cast<std::uint64_t>(source.GetIndex()), m_sourceCardDbfID,
      {{target.GetDbfID(), static_cast<std::uint64_t>(target.GetIndex())}});
  return TaskStatus::COMPLETE;
}
}
