#include <Rosetta/Battlegrounds/Models/Season14.hpp>

#include <algorithm>
#include <utility>

namespace RosettaStone::Battlegrounds
{
void Season14State::BeginDecision(
    Season14Decision decision, std::vector<Season14Offering> offerings)
{
    if (decision == Season14Decision::NONE)
    {
        pendingDecision = Season14Decision::NONE;
        choiceOfferings.clear();
        pendingOfferings.clear();
        return;
    }

    pendingDecision = decision;
    pendingOfferings = std::move(offerings);
    if (decision == Season14Decision::CHOICE ||
        decision == Season14Decision::DISCOVER)
    {
        choiceOfferings = pendingOfferings;
    }
    else
    {
        choiceOfferings.clear();
    }
}

bool Season14State::SelectDecision(std::size_t offeringIndex)
{
    if (pendingDecision == Season14Decision::NONE ||
        offeringIndex >= pendingOfferings.size())
    {
        return false;
    }

    choiceOfferings.clear();
    pendingOfferings.clear();
    pendingDecision = Season14Decision::NONE;
    return true;
}

void Season14State::SetHeroPower(std::int32_t dbfID, std::int32_t cost,
                                 bool available)
{
    heroPowerDbfID = dbfID;
    heroPowerCost = std::max<std::int32_t>(0, cost);
    heroPowerAvailable = available;
    heroPowerUsed = false;
}

bool Season14State::CanUseHeroPower(std::int32_t availableGold) const
{
    return heroPowerAvailable && !heroPowerUsed && heroPowerDbfID != 0 &&
           availableGold >= heroPowerCost;
}

bool Season14State::UseHeroPower()
{
    if (!heroPowerAvailable || heroPowerUsed || heroPowerDbfID == 0)
    {
        return false;
    }

    heroPowerUsed = true;
    return true;
}

bool Season14State::CanAddTrinket() const
{
    return trinkets.size() < SEASON14_TRINKET_SLOTS;
}

bool Season14State::CanAddDarkGift() const
{
    return darkGifts.size() < SEASON14_DARK_GIFT_SLOTS;
}

void Season14State::AddTrinket(Season14PersistentEffect effect)
{
    if (CanAddTrinket())
    {
        trinkets.push_back(effect);
    }
}

void Season14State::AddDarkGift(Season14PersistentEffect effect)
{
    if (CanAddDarkGift())
    {
        darkGifts.push_back(effect);
    }
}

bool Season14State::ConsumeEffect(
    std::vector<Season14PersistentEffect>& effects, std::size_t slot)
{
    if (slot >= effects.size() || !effects[slot].active)
    {
        return false;
    }

    if (effects[slot].remainingUses > 0)
    {
        --effects[slot].remainingUses;
        if (effects[slot].remainingUses == 0)
        {
            effects[slot].active = false;
        }
    }
    return true;
}

void Season14State::Emit(Season14Event event)
{
    const auto index = static_cast<std::size_t>(event);
    if (index < eventCounts.size())
    {
        ++eventCounts[index];
    }
}

bool Season14State::IsValidBoardTarget(std::int32_t index,
                                       std::int32_t boardCount)
{
    return index >= 0 && index < boardCount;
}
}  // namespace RosettaStone::Battlegrounds
