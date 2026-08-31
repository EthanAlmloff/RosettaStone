#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyHighestHandMurlocBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus RallyHighestHandMurlocBuffTask::Run(Player& p, Minion& source) { int a=0,h=0; p.hand.ForEach([&](const std::optional<CardData>& e){if(std::holds_alternative<Minion>(*e)){const auto& m=std::get<Minion>(*e);if(m.GetAttack()>a){a=m.GetAttack();h=m.GetHealth();}}}); int n=0; p.recruitField.ForEachAlive([&](MinionData& d){auto& m=d.value(); if(&m!=&source && m.HasRace(Race::MURLOC) && n<2){m.SetAttack(m.GetAttack()+a*m_repeats);m.SetHealth(m.GetHealth()+h*m_repeats);++n;}}); return TaskStatus::COMPLETE; }
TaskStatus RallyHighestHandMurlocBuffTask::Run(Player& p, Minion& s, Minion&) { return Run(p,s); }
}
