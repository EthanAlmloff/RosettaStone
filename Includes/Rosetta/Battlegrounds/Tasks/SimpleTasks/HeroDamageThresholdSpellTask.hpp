#ifndef ROSETTASTONE_BATTLEGROUNDS_HERO_DAMAGE_THRESHOLD_SPELL_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HERO_DAMAGE_THRESHOLD_SPELL_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
#include <utility>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class HeroDamageThresholdSpellTask { public:
 HeroDamageThresholdSpellTask(int threshold,std::string spellID,int amount):m_threshold(threshold),m_spellID(std::move(spellID)),m_amount(amount){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_threshold; std::string m_spellID; int m_amount;
}; }}
#endif
