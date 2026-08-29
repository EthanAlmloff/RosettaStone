// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Models/Spell.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
Spell::Spell(Card card) : m_card(std::move(card))
{
    // Do nothing
}

int Spell::GetDbfID() const
{
    return m_card.dbfID;
}

int Spell::GetCost() const
{
    const auto cost = m_card.gameTags.find(GameTag::COST);
    return cost == m_card.gameTags.end() ? 0 : cost->second;
}

const std::string& Spell::GetID() const
{
    return m_card.id;
}

const std::string& Spell::GetText() const
{
    return m_card.text;
}

ZoneType Spell::GetZoneType() const
{
    return m_zoneType;
}

void Spell::SetZoneType(ZoneType type)
{
    m_zoneType = type;
}

int Spell::GetZonePosition() const
{
    return m_zonePos;
}

void Spell::SetZonePosition(int pos)
{
    m_zonePos = pos;
}
}  // namespace RosettaStone::Battlegrounds
