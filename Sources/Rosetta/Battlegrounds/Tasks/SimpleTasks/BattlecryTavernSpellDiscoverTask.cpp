// Copyright (c) 2026 Hearthstone BG AI contributors
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellDiscoverTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus BattlecryTavernSpellDiscoverTask::Run(Player& p, Minion& s) {
 return p.BeginTavernSpellDiscover(m_amount, static_cast<std::uint64_t>(s.GetIndex()), s.GetDbfID()) ? TaskStatus::COMPLETE : TaskStatus::STOP;
}
TaskStatus BattlecryTavernSpellDiscoverTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
