#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class MechDiscoverMagnetizeTask { public: explicit MechDiscoverMagnetizeTask(int count):m_count(count){} TaskStatus Run(Player&,Minion&,Minion&); TaskStatus Run(Player&,Minion&); int Count()const noexcept{return m_count;} private:int m_count;};
}}
