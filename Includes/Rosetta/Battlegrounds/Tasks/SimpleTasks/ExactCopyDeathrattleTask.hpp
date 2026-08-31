#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <cstddef>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class ExactCopyDeathrattleTask { public: explicit ExactCopyDeathrattleTask(std::size_t snapshotId):m_snapshotId(snapshotId){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: std::size_t m_snapshotId; };
}}
