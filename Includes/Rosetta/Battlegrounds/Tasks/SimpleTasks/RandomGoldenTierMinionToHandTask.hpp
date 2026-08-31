// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_RANDOM_GOLDEN_TIER_MINION_TO_HAND_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_RANDOM_GOLDEN_TIER_MINION_TO_HAND_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class RandomGoldenTierMinionToHandTask { public:
 explicit RandomGoldenTierMinionToHandTask(int amount):m_amount(amount){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 int Amount() const noexcept { return m_amount; }
 private:int m_amount=0; }; } }
#endif
