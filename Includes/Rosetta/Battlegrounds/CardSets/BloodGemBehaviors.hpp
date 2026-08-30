// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_BLOOD_GEM_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BLOOD_GEM_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::Battlegrounds
{
//! Registers only Blood Gem cards whose complete runtime behavior is modeled.
//! Cards with incomplete recursive triggers (Aggem) intentionally remain
//! unregistered.
class BloodGemBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_BLOOD_GEM_BEHAVIORS_HPP
