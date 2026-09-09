#pragma once

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::Battlegrounds
{
//! Season 14 Buddy entities whose effect is resolved at the spell-target
//! boundary rather than by a static task graph.
class ModernTokenBehaviorsBatch77
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}
