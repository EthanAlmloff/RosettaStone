#ifndef ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

namespace RosettaStone::Battlegrounds
{
//! Explicit Patch 36.4 Activate registrations with complete simple effects.
class ActivateBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif
