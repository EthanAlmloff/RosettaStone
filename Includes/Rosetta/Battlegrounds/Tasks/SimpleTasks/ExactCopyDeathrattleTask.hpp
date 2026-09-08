#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
#include <cstddef>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class ExactCopyDeathrattleTask { public: explicit ExactCopyDeathrattleTask(std::size_t snapshotId, int count = 1, bool fullHealth = false):m_snapshotId(snapshotId),m_count(count),m_fullHealth(fullHealth){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private: std::size_t m_snapshotId; int m_count = 1; bool m_fullHealth = false; };
}}
