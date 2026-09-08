// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/TargetingPredicates.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>

#include <stdexcept>

namespace RosettaStone::Battlegrounds
{
TargetingPredicate TargetingPredicates::ReqMurlocTarget()
{
    return [](Minion& minion) { return minion.GetRace() == Race::MURLOC; };
}

TargetingPredicate TargetingPredicates::ReqTargetWithRace(Race race)
{
    // The predicate is a generic metadata restriction.  Keeping the
    // implementation generic avoids making startup depend on whether a
    // particular card's tribe has a bespoke targeting helper.
    return [race](Minion& minion) { return minion.HasRace(race); };
}
}  // namespace RosettaStone::Battlegrounds
