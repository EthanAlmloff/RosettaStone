#ifndef ROSETTASTONE_BATTLEGROUNDS_ADD_CARD_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ADD_CARD_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <string>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class AddCardTask { public: AddCardTask(std::string cardID,int amount); TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); const std::string& CardID()const noexcept{return m_cardID;} int Amount()const noexcept{return m_amount;} private: std::string m_cardID; int m_amount; };
} }
#endif
