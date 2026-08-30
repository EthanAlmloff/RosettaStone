#pragma once

#include <map>
#include <string>

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

namespace RosettaStone::Battlegrounds
{
//! Sprightly Scarab's modal is applied transactionally by Player::ApplyChooseOne.
//! These four identities are registered so the bridge/coverage manifest can
//! distinguish the implemented runtime family from absent card content.
class ModernMinionBehaviorsBatch36
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds
