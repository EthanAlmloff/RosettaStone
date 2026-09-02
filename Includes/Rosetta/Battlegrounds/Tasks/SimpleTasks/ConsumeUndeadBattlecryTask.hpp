#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class ConsumeUndeadBattlecryTask { public: ConsumeUndeadBattlecryTask(bool discover,int copies):m_discover(discover),m_copies(copies){} TaskStatus Run(Player&,Minion&,Minion&); TaskStatus Run(Player&,Minion&); int Copies()const noexcept{return m_copies;} bool Discover()const noexcept{return m_discover;} private:bool m_discover;int m_copies;};
}}
