#pragma once

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>

namespace RosettaStone::Battlegrounds {
//! Season 14 buddy/generated-token registrations reviewed in batch 71.
class ModernTokenBehaviorsBatch71 {
 public:
  static void AddAll(std::map<std::string, CardDef>& cards);
};
}
