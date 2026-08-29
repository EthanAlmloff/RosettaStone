// Copyright (c) 2026 HSBGML contributors
//
// Header-only assembly for the pinned Patch 36.4 hero/power metadata view.
// CardLoader remains the source of card metadata; this helper only selects the
// active registry and validates links. It intentionally registers no effects.

#ifndef ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_METADATA_REGISTRY_HPP
#define ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_METADATA_REGISTRY_HPP

#include <Rosetta/Battlegrounds/Cards/Card.hpp>
#include <Rosetta/Battlegrounds/Cards/HeroPowerRegistry.hpp>
#include <Rosetta/Common/Constants.hpp>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>

namespace RosettaStone::Battlegrounds::HeroPowerRegistry
{
//! Select and validate metadata-only powers from CardLoader's full snapshot.
//!
//! The caller supplies the current-hero view so this function cannot infer
//! hidden or stale links from mutable simulator state. A missing link, count,
//! replacement, or behavior definition throws instead of silently degrading.
inline std::array<Card, EXPECTED_POWER_COUNT> BuildMetadataRegistry(
    const std::array<Card, NUM_BATTLEGROUNDS_CARDS>& cards,
    const std::array<Card, NUM_BATTLEGROUNDS_HEROES>& heroes)
{
    std::array<Card, EXPECTED_POWER_COUNT> result{};
    const auto active_heroes = std::count_if(
        heroes.begin(), heroes.end(), [](const Card& hero) {
            return !hero.id.empty();
        });
    if (active_heroes != EXPECTED_HERO_COUNT)
    {
        throw std::logic_error(
            "hero metadata registry expected " +
            std::to_string(EXPECTED_HERO_COUNT) + " active heroes, found " +
            std::to_string(active_heroes));
    }

    std::size_t power_index = 0;
    for (const auto& card : cards)
    {
        if (card.id.empty() || card.GetCardType() != CardType::HERO_POWER)
        {
            continue;
        }
        const bool linked = std::any_of(
            heroes.begin(), heroes.end(), [&card](const Card& hero) {
                return !hero.id.empty() && hero.heroPowerDbfID != 0 &&
                       hero.heroPowerDbfID == card.dbfID;
            });
        if (!linked && !IsGeneratedReplacement(card.id))
        {
            continue;
        }
        if (card.hasBehavior)
        {
            throw std::logic_error(
                "metadata-only hero power unexpectedly has behavior: " +
                card.id);
        }
        if (power_index == result.size())
        {
            throw std::length_error("hero-power metadata registry overflow");
        }
        result.at(power_index++) = card;
    }

    if (power_index != result.size())
    {
        throw std::logic_error(
            "hero metadata registry expected " +
            std::to_string(result.size()) + " powers, found " +
            std::to_string(power_index));
    }

    for (const auto& hero : heroes)
    {
        if (hero.id.empty())
        {
            continue;
        }
        if (hero.heroPowerDbfID == 0 ||
            std::none_of(result.begin(), result.end(), [&hero](const Card& power) {
                return power.dbfID == hero.heroPowerDbfID;
            }))
        {
            throw std::logic_error(
                "hero-power DBF link does not resolve for hero: " + hero.id);
        }
    }
    return result;
}
}  // namespace RosettaStone::Battlegrounds::HeroPowerRegistry

#endif  // ROSETTASTONE_BATTLEGROUNDS_HERO_POWER_METADATA_REGISTRY_HPP
