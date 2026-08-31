// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_TAVERN_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TAVERN_HPP

#include <Rosetta/Battlegrounds/Zones/FieldZone.hpp>
#include <Rosetta/Battlegrounds/Models/Spell.hpp>

#include <variant>
#include <cstddef>
#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
//! A discriminated Tavern entry.  The legacy FieldZone remains as a
//! minion-only adapter while spell-bearing Tavern slots migrate callers.
class TavernSlot
{
 public:
    using Value = std::variant<Minion, Spell>;
    explicit TavernSlot(Minion value) : m_value(std::move(value)) {}
    explicit TavernSlot(Spell value) : m_value(std::move(value)) {}
    bool IsMinion() const noexcept { return std::holds_alternative<Minion>(m_value); }
    bool IsSpell() const noexcept { return std::holds_alternative<Spell>(m_value); }
    Minion& AsMinion() { return std::get<Minion>(m_value); }
    const Minion& AsMinion() const { return std::get<Minion>(m_value); }
    Spell& AsSpell() { return std::get<Spell>(m_value); }
    const Spell& AsSpell() const { return std::get<Spell>(m_value); }
    bool IsFrozen() const noexcept
    {
        return IsMinion() ? AsMinion().IsFrozen() : m_frozen;
    }
    void SetFrozen(bool value) noexcept
    {
        if (IsMinion()) AsMinion().SetFrozen(value);
        else m_frozen = value;
    }
 private:
    Value m_value;
    bool m_frozen = false;
};

//!
//! \brief Tavern class.
//!
//! This class represents the Tavern that used in Battlegrounds.
//!
class Tavern
{
 public:
    FieldZone fieldZone;
    //! Spell-bearing slots are separate during the adapter migration; no
    //! spell is silently represented as a minion.
    std::vector<TavernSlot> spellSlots;
    void ClearSpellSlots() { spellSlots.clear(); }
    std::size_t SlotCount() const noexcept
    {
        return static_cast<std::size_t>(fieldZone.GetCount()) + spellSlots.size();
    }
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TAVERN_HPP
