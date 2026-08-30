// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH10_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH10_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::Battlegrounds
{
//! Verified Patch 36.4 end-of-turn minion behavior family.
class ModernMinionBehaviorsBatch10
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH10_HPP
