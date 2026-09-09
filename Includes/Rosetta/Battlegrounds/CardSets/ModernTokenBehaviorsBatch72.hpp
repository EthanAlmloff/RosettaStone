#pragma once

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::Battlegrounds {
//! Season 14 token/golden registrations reviewed in batch 72.
class ModernTokenBehaviorsBatch72 {
 public:
  static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds
