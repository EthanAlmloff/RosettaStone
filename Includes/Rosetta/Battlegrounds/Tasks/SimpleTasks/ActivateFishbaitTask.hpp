#ifndef ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_FISHBAIT_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_FISHBAIT_TASK_HPP
#include <string>
#include <utility>
#include <cstddef>
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class ActivateFishbaitTask { public: ActivateFishbaitTask(std::string id,int stat):m_id(std::move(id)),m_stat(stat){} TaskStatus Run(Player&,Minion&); TaskStatus RunAt(Player&,Minion&,std::size_t); TaskStatus Run(Player&,Minion&,Minion&); private: std::string m_id; int m_stat; };
}}
#endif
