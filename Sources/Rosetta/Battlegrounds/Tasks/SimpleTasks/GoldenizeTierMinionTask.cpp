#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus GoldenizeTierMinionTask::Run(Player& player, Minion& source, Minion& target) {
  const bool buddyOnly = source.GetCardID() == "BG25_HERO_105_Buddy" ||
                         source.GetCardID() == "BG25_HERO_105_Buddy_G";
  const auto isBuddy = [](const Minion& minion) {
    return minion.GetCardID().find("_Buddy") != std::string::npos;
  };
  if (m_count <= 0 || target.IsDestroyed() || target.GetTier() > 6 ||
      !target.CanMakeGolden() || (buddyOnly && !isBuddy(target)))
    return TaskStatus::STOP;
  if (m_count > 1)
  {
    player.season14.pendingGoldenizeTargets.clear();
    player.GetField().ForEachAlive([&](MinionData& data) {
      auto& candidate = data.value();
      if (&candidate != &source && &candidate != &target &&
          candidate.GetTier() <= 6 && candidate.CanMakeGolden() &&
          (!buddyOnly || isBuddy(candidate)))
        player.season14.pendingGoldenizeTargets.push_back(
            static_cast<std::uint64_t>(candidate.GetIndex()));
    });
    if (player.season14.pendingGoldenizeTargets.empty()) return TaskStatus::STOP;
    player.season14.pendingGoldenizeSourceEntityID =
        static_cast<std::uint64_t>(source.GetIndex());
    player.season14.pendingGoldenizeFirstTargetEntityID =
        static_cast<std::uint64_t>(target.GetIndex());
    player.season14.pendingDecision = Season14Decision::CHOICE;
    return TaskStatus::COMPLETE;
  }
  const int made = target.MakeGolden() ? 1 : 0;
  return made == 0 ? TaskStatus::STOP : TaskStatus::COMPLETE;
}
TaskStatus GoldenizeTierMinionTask::Run(Player&, Minion&) { return TaskStatus::STOP; }
}
