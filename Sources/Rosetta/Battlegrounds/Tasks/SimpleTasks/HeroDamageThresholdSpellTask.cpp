#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HeroDamageThresholdSpellTask.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus HeroDamageThresholdSpellTask::Run(Player& p,Minion& s){int n=p.season14.heroDamageThisTurn;if(n<m_threshold){s.SetHeroDamageThresholdFired(false);return TaskStatus::COMPLETE;}if(s.HeroDamageThresholdFired())return TaskStatus::COMPLETE;if(p.CastTavernSpellFree(m_spellID,m_amount))s.SetHeroDamageThresholdFired(true);return TaskStatus::COMPLETE;}
TaskStatus HeroDamageThresholdSpellTask::Run(Player&p,Minion&s,Minion&){return Run(p,s);}
}
