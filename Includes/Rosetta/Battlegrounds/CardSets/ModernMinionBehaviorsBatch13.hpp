#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH13_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH13_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::Battlegrounds
{
//! Verified simple Battlecry gold effects from the pinned Patch 36.4 pool.
class ModernMinionBehaviorsBatch13
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif
