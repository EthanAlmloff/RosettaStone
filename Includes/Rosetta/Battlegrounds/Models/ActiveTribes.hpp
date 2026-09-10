// Copyright (c) 2024
//
// Pinned lobby tribe eligibility for the Patch 36.4 solo ruleset.  Keep this
// data separate from Card metadata: a card can carry a tribe tag while its
// tribe is not offered by the current lobby.
#ifndef ROSETTASTONE_BATTLEGROUNDS_ACTIVE_TRIBES_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ACTIVE_TRIBES_HPP

#include <Rosetta/Common/Constants.hpp>
#include <Rosetta/Battlegrounds/Cards/Card.hpp>

#include <array>
#include <algorithm>
#include <cstdint>

namespace RosettaStone::Battlegrounds
{
using ActiveTribeSet = std::array<Race, RACES_IN_BATTLEGROUNDS.size()>;

// The manifest lists the ten eligible tribe universe.  A normal solo lobby
// exposes five of those tribes; the concrete five are selected once from the
// lobby seed and copied into GameState.  Keeping those concepts separate is
// important: a manifest is not evidence that every tribe is in one lobby.
inline constexpr ActiveTribeSet PINNED_ACTIVE_TRIBES =
    RACES_IN_BATTLEGROUNDS;

inline constexpr std::size_t ACTIVE_TRIBES_PER_LOBBY = 5;

inline ActiveTribeSet SelectActiveTribes(std::uint64_t seed)
{
    ActiveTribeSet selected{};
    selected.fill(Race::INVALID);
    // Fisher-Yates over the ten eligible races using a tiny local PRNG.  This
    // avoids consuming Rosetta's gameplay RNG stream and makes lobby tribe
    // selection a pure function of the explicit lobby seed.
    std::array<std::size_t, RACES_IN_BATTLEGROUNDS.size()> order{};
    for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::uint64_t state = seed ^ 0x9e3779b97f4a7c15ULL;
    const auto next = [&state]() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 0x2545F4914F6CDD1DULL;
    };
    for (std::size_t i = order.size(); i > 1; --i) {
        const auto j = static_cast<std::size_t>(next() % i);
        std::swap(order[i - 1], order[j]);
    }
    for (std::size_t i = 0; i < ACTIVE_TRIBES_PER_LOBBY; ++i)
        selected[i] = RACES_IN_BATTLEGROUNDS[order[i]];
    return selected;
}

inline constexpr std::uint32_t ActiveTribeBits(const ActiveTribeSet& active)
{
    std::uint32_t bits = 0;
    for (std::size_t i = 0; i < RACES_IN_BATTLEGROUNDS.size(); ++i)
        if (std::find(active.begin(), active.end(), RACES_IN_BATTLEGROUNDS[i]) !=
            active.end())
            bits |= (std::uint32_t{1} << i);
    return bits;
}

inline constexpr bool IsActiveTribe(const ActiveTribeSet& active, Race race)
{
    if (race == Race::ALL) return true;
    return std::find(active.begin(), active.end(), race) != active.end();
}

inline bool HasActiveTribe(const ActiveTribeSet& active, const Card& card)
{
    if (card.GetCardType() != CardType::MINION) return true;
    for (const auto race : RACES_IN_BATTLEGROUNDS)
        if (card.HasRace(race) && IsActiveTribe(active, race)) return true;
    // Untyped minions (and metadata rows with no race) remain legal.  A
    // concrete non-Battlegrounds tribe is fail-closed rather than leaking
    // into an ordinary pool.
    return card.GetRace() == Race::INVALID || card.GetRace() == Race::ALL;
}
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_ACTIVE_TRIBES_HPP
