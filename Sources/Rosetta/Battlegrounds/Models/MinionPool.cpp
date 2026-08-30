// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/MinionPool.hpp>
#include <Rosetta/Battlegrounds/Utils/GameUtils.hpp>

#include <effolkronium/random.hpp>

#include <stdexcept>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
void MinionPool::Initialize(Race excludeRace)
{
    std::size_t idx = 0;

    // Tier 1
    for (const auto& card : Cards::GetTier1Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER1_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 2
    for (const auto& card : Cards::GetTier2Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER2_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 3
    for (const auto& card : Cards::GetTier3Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER3_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 4
    for (const auto& card : Cards::GetTier4Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER4_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 5
    for (const auto& card : Cards::GetTier5Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER5_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 6
    for (const auto& card : Cards::GetTier6Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER6_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    // Tier 7
    for (const auto& card : Cards::GetTier7Minions())
    {
        if (card.id.empty())
        {
            continue;
        }
        if (!card.hasBehavior)
        {
            throw std::invalid_argument("missing Battlegrounds behavior for pool card: " + card.id);
        }
        if (card.GetRace() == excludeRace)
        {
            continue;
        }

        for (std::size_t i = 0; i < NUM_COPIES_OF_EACH_TIER7_MINIONS; ++i)
        {
            m_minions.at(idx) = { Minion(card, idx), idx, true };
            ++idx;
        }
    }

    m_count = idx;
}

void MinionPool::InitializeSupported(const std::vector<std::string>& cardIDs)
{
    if (cardIDs.empty())
    {
        throw std::invalid_argument("supported minion pool cannot be empty");
    }

    std::vector<Card> cards;
    cards.reserve(cardIDs.size());
    for (const auto& id : cardIDs)
    {
        Card card = Cards::FindCardByID(id);
        if (card.id.empty() || card.GetCardType() != CardType::MINION ||
            card.GetTier() < 1)
        {
            throw std::invalid_argument("unsupported minion pool card ID: " +
                                        id);
        }
        cards.emplace_back(std::move(card));
    }

    for (std::size_t idx = 0; idx < m_minions.size(); ++idx)
    {
        const Card& card = cards.at(idx % cards.size());
        m_minions.at(idx) = { Minion(card, static_cast<int>(idx)),
                              static_cast<int>(idx), true };
    }
    m_count = m_minions.size();
}

std::size_t MinionPool::GetCount() const
{
    return m_count;
}

void MinionPool::AddMinionsToTavern(Player& player, Tavern& tavern)
{
    const std::size_t targetCount = player.season14.TavernOfferCount(
        GetNumMinionsCanPurchase(player.currentTier));
    const std::size_t currentCount =
        static_cast<std::size_t>(tavern.fieldZone.GetCount());
    if (currentCount >= targetCount)
    {
        return;
    }
    const std::size_t numMinions = targetCount - currentCount;
    auto minions = GetMinions(1, player.currentTier, true);

    Random::shuffle(minions.begin(), minions.end());

    // Temporal Tavern arms exactly one subsequent fill. Select the requested
    // higher-tier offers first, then fill the remaining slots normally. The
    // allowance is consumed here, after the refresh has been accepted, so a
    // failed/unsupported action cannot leave a stale bonus for a later turn.
    const auto requestedHigher = player.season14.TakeHigherTierRefresh();
    if (requestedHigher > 0 && player.currentTier < TIER_UPPER_LIMIT)
    {
        auto higherTier = GetMinions(player.currentTier + 1,
                                     player.currentTier + 1, true);
        Random::shuffle(higherTier.begin(), higherTier.end());
        const auto higherCount = std::min<std::size_t>(
            static_cast<std::size_t>(requestedHigher),
            std::min(targetCount - currentCount, higherTier.size()));
        minions.insert(minions.begin(), higherTier.begin(),
                       higherTier.begin() + static_cast<std::ptrdiff_t>(higherCount));
    }

    std::size_t idx = 0;
    for (auto& minion : minions)
    {
        // A passive such as ALL Will Burn! applies when a fresh minion
        // instance is created, including fills that bypass Player::PrepareTavern
        // in isolated bridge/replay setups.
        player.ApplyFreshMinionModifiers(minion);
        tavern.fieldZone.Add(minion);
        std::get<2>(m_minions.at(minion.GetPoolIndex())) = false;
        ++idx;

        if (idx == numMinions)
        {
            break;
        }
    }
}

void MinionPool::ReturnMinion(int idx)
{
    if (idx < 0 || idx >= NUM_TOTAL_TAVERN_MINIONS)
    {
        return;
    }

    std::get<2>(m_minions.at(idx)) = true;
}

std::vector<Minion> MinionPool::GetMinions(int minTier, int maxTier,
                                           bool isInPoolOnly)
{
    std::vector<Minion> result;

    for (auto& minion : m_minions)
    {
        const int tier = std::get<0>(minion).GetTier();

        if (tier >= minTier && tier <= maxTier)
        {
            if (isInPoolOnly && std::get<2>(minion) == false)
            {
                continue;
            }

            result.emplace_back(std::get<0>(minion));
        }
    }

    return result;
}
}  // namespace RosettaStone::Battlegrounds
