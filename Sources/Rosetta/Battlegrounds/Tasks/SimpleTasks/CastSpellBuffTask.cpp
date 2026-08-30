#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastSpellBuffTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus CastSpellBuffTask::Run(Player&,Minion&) { return TaskStatus::COMPLETE; }
TaskStatus CastSpellBuffTask::Run(Player& p,Minion&,Minion&) { if (m_mode) p.ApplySpellSpecialBuff(m_mode,m_attack,m_health); else p.ApplySpellRaceBuff(m_race,m_attack,m_health,m_includeHand); return TaskStatus::COMPLETE; }
}
