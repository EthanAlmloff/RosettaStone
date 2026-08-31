#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRandomRaceKeywordTask.hpp>
#include <effolkronium/random.hpp>
#include <algorithm>
#include <vector>
using Random = effolkronium::random_thread_local;
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyRandomRaceKeywordTask::Run(Player& player, Minion& source) {
  std::vector<Minion*> candidates;
  player.GetField().ForEachAlive([&](MinionData& data) { auto& m=data.value(); if (&m != &source && (m_race == Race::ALL || m.HasRace(m_race))) candidates.push_back(&m); });
  if (candidates.empty() || m_amount <= 0) return candidates.empty() ? TaskStatus::STOP : TaskStatus::COMPLETE;
  Random::shuffle(candidates.begin(), candidates.end());
  const auto count=std::min<std::size_t>(static_cast<std::size_t>(m_amount), candidates.size());
  for (std::size_t i=0;i<count;++i) candidates[i]->SetGameTag(m_tag, 1);
  return TaskStatus::COMPLETE;
}
TaskStatus RallyRandomRaceKeywordTask::Run(Player& player, Minion& source, Minion&) { return Run(player, source); }
}
