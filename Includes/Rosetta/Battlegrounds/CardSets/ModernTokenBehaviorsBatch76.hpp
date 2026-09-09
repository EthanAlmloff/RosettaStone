#pragma once

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>

namespace RosettaStone::Battlegrounds {
class ModernTokenBehaviorsBatch76 {
 public:
  static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds
