#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyHighestHandSummonTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyHighestHandSummonTask::Run(Player& p, Minion&) { const Minion* best=nullptr; p.hand.ForEach([&](const std::optional<CardData>& e){if(std::holds_alternative<Minion>(*e)){const auto& m=std::get<Minion>(*e);if(!best||m.GetAttack()>best->GetAttack())best=&m;}}); if(!best)return TaskStatus::STOP; p.season14.ArmCombatHandSummon(*best,m_count); return TaskStatus::COMPLETE; }
TaskStatus RallyHighestHandSummonTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
