// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>

#include <utility>
#include <vector>

namespace RosettaStone::Battlegrounds
{
Minion::Minion(Card card, int poolIdx)
    : m_card(std::move(card)),
      m_poolIdx(poolIdx),
      m_attack(m_card.GetAttack()),
      m_health(m_card.GetHealth())
{
    for (const auto& tag : m_card.gameTags)
    {
        switch (tag.first)
        {
            case GameTag::DEATHRATTLE:
                m_hasDeathrattle = true;
                break;
            case GameTag::TAUNT:
                m_hasTaunt = true;
                break;
            case GameTag::DIVINE_SHIELD:
                m_hasDivineShield = true;
                break;
            case GameTag::REBORN:
                m_hasReborn = true;
                break;
            case GameTag::WINDFURY:
                m_hasWindfury = true;
                break;
            case GameTag::MEGA_WINDFURY:
                m_hasMegaWindfury = true;
                break;
            case GameTag::POISONOUS:
            case GameTag::VENOMOUS:
                m_hasVenomous = true;
                break;
            case GameTag::STEALTH:
                m_hasStealth = true;
                break;
            default:
                break;
        }
    }
}

int Minion::GetIndex() const
{
    return m_index;
}

void Minion::SetIndex(int index)
{
    m_index = index;
}

int Minion::GetPoolIndex() const
{
    return m_poolIdx;
}

std::string_view Minion::GetName() const
{
    return m_card.name;
}

std::string_view Minion::GetCardID() const
{
    return m_card.id;
}

int Minion::GetDbfID() const
{
    return m_card.dbfID;
}

int Minion::GetGameTag(GameTag tag) const
{
    switch (tag)
    {
        case GameTag::ATK:
            return GetAttack();
        case GameTag::TAUNT:
            return HasTaunt() ? 1 : 0;
        case GameTag::DIVINE_SHIELD:
            return HasDivineShield() ? 1 : 0;
        default:
            return 0;
    }
}

void Minion::SetGameTag(GameTag tag, int value)
{
    switch (tag)
    {
        case GameTag::TAUNT:
            m_hasTaunt = value == 1;
            break;
        case GameTag::DIVINE_SHIELD:
            m_hasDivineShield = value == 1 ? true : false;
            break;
        case GameTag::WINDFURY:
            m_hasWindfury = value == 1 ? true : false;
            break;
        case GameTag::MEGA_WINDFURY:
            m_hasMegaWindfury = value == 1 ? true : false;
            break;
        case GameTag::POISONOUS:
        case GameTag::VENOMOUS:
            m_hasVenomous = value == 1 ? true : false;
            break;
        case GameTag::STEALTH:
            m_hasStealth = value == 1 ? true : false;
            break;
        default:
            break;
    }
}

Race Minion::GetRace() const
{
    return m_card.GetRace();
}

bool Minion::HasRace(Race race) const
{
    return m_card.HasRace(race);
}

ZoneType Minion::GetZoneType() const
{
    return m_zoneType;
}

void Minion::SetZoneType(ZoneType type)
{
    m_zoneType = type;
}

int Minion::GetZonePosition() const
{
    return m_zonePos;
}

void Minion::SetZonePosition(int pos)
{
    m_zonePos = pos;
}

int Minion::GetLastFieldPos() const
{
    return m_lastFieldPos;
}

void Minion::SetLastFieldPos(int pos)
{
    m_lastFieldPos = pos;
}

int Minion::GetTier() const
{
    return m_card.GetTier();
}

bool Minion::IsGolden() const
{
    // HearthstoneJSON links a golden entity back to its normal DBF ID.  This
    // remains stable for generated/pool minions and avoids guessing from
    // names or card text.
    return m_card.normalDbfID != 0;
}

bool Minion::MakeGolden()
{
    if (!CanMakeGolden())
    {
        return false;
    }

    Card premium = Cards::FindCardByDbfID(m_card.premiumDbfID);

    // The premium entity supplies the golden card identity/keywords while
    // the live instance keeps its current stats and zone/index state.  This
    // mirrors the in-game conversion of a buffed Tavern minion and avoids
    // turning this spell into a metadata-only flag.
    const int currentAttack = m_attack;
    const int currentHealth = m_health;
    const int globalMinionAttack = m_globalMinionAttack;
    // The premium card supplies the new identity and its static keywords,
    // but conversion must not erase state accumulated by this particular
    // instance.  Dark Gifts and other recruit effects mutate these fields
    // without changing Card::gameTags, so snapshot them before replacing the
    // card metadata.
    const bool hadDeathrattle = m_hasDeathrattle;
    const bool hadTaunt = m_hasTaunt;
    const bool hadDivineShield = m_hasDivineShield;
    const bool hadReborn = m_hasReborn;
    const bool hadWindfury = m_hasWindfury;
    const bool hadMegaWindfury = m_hasMegaWindfury;
    const bool hadVenomous = m_hasVenomous;
    const bool hadStealth = m_hasStealth;
    const bool wasFrozen = m_isFrozen;
    const bool wasDestroyed = m_isDestroyed;
    const int startCombatAttackMultiplier = m_startCombatAttackMultiplier;
    const int startCombatHealthMultiplier = m_startCombatHealthMultiplier;
    const bool startCombatStatsApplied = m_startCombatStatsApplied;
    m_card = std::move(premium);
    m_card.Initialize();
    m_attack = currentAttack;
    m_health = currentHealth;
    m_globalMinionAttack = globalMinionAttack;

    m_hasDeathrattle = false;
    m_hasTaunt = false;
    m_hasDivineShield = false;
    m_hasReborn = false;
    m_hasWindfury = false;
    m_hasMegaWindfury = false;
    m_hasVenomous = false;
    m_hasStealth = false;
    for (const auto& tag : m_card.gameTags)
    {
        switch (tag.first)
        {
            case GameTag::DEATHRATTLE:
                m_hasDeathrattle = true;
                break;
            case GameTag::TAUNT:
                m_hasTaunt = true;
                break;
            case GameTag::DIVINE_SHIELD:
                m_hasDivineShield = true;
                break;
            case GameTag::REBORN:
                m_hasReborn = true;
                break;
            case GameTag::WINDFURY:
                m_hasWindfury = true;
                break;
            case GameTag::MEGA_WINDFURY:
                m_hasMegaWindfury = true;
                break;
            case GameTag::POISONOUS:
            case GameTag::VENOMOUS:
                m_hasVenomous = true;
                break;
            case GameTag::STEALTH:
                m_hasStealth = true;
                break;
            default:
                break;
        }
    }

    // Preserve the exact runtime keyword state previously accumulated by
    // this instance.  In particular, OR-ing the premium metadata back in
    // would incorrectly restore a Reborn charge that had already been
    // consumed before Gilding.  Normal and premium entities carry the same
    // static keyword set in the supported card data; the mutable fields below
    // are therefore authoritative for the converted instance.
    m_hasDeathrattle = hadDeathrattle;
    m_hasTaunt = hadTaunt;
    m_hasDivineShield = hadDivineShield;
    m_hasReborn = hadReborn;
    m_hasWindfury = hadWindfury;
    m_hasMegaWindfury = hadMegaWindfury;
    m_hasVenomous = hadVenomous;
    m_hasStealth = hadStealth;
    m_isFrozen = wasFrozen;
    m_isDestroyed = wasDestroyed;
    m_startCombatAttackMultiplier = startCombatAttackMultiplier;
    m_startCombatHealthMultiplier = startCombatHealthMultiplier;
    m_startCombatStatsApplied = startCombatStatsApplied;
    return true;
}

bool Minion::CanMakeGolden() const
{
    if (IsGolden() || m_card.premiumDbfID == 0)
    {
        return false;
    }

    return !Cards::FindCardByDbfID(m_card.premiumDbfID).id.empty();
}

int Minion::GetAttack() const
{
    return m_attack;
}

void Minion::SetAttack(int val)
{
    m_attack = val;
}

void Minion::ApplyGlobalMinionAttack(int attack)
{
    if (attack <= m_globalMinionAttack)
    {
        return;
    }

    m_attack += attack - m_globalMinionAttack;
    m_globalMinionAttack = attack;
}

int Minion::GetGlobalMinionAttack() const
{
    return m_globalMinionAttack;
}

int Minion::GetHealth() const
{
    return m_health;
}

void Minion::SetHealth(int val)
{
    m_health = val;
}

bool Minion::HasDeathrattle() const
{
    return m_hasDeathrattle;
}

bool Minion::HasTaunt() const
{
    return m_hasTaunt;
}

bool Minion::HasDivineShield() const
{
    return m_hasDivineShield;
}

bool Minion::HasReborn() const
{
    return m_hasReborn;
}

void Minion::SetReborn(bool reborn)
{
    m_hasReborn = reborn;
}

bool Minion::HasWindfury() const
{
    return m_hasWindfury || m_hasMegaWindfury;
}

bool Minion::HasVenomous() const
{
    return m_hasVenomous;
}

bool Minion::HasStealth() const
{
    return m_hasStealth;
}

void Minion::SetStartCombatStatMultipliers(int attackMultiplier,
                                           int healthMultiplier)
{
    if (attackMultiplier < 1 || healthMultiplier < 1)
    {
        return;
    }
    // Multiple start-of-combat Dark Gifts compose.  Replacing the previous
    // multiplier made Resistance + Hostility depend on application order and
    // silently discarded one of the gifts.
    m_startCombatAttackMultiplier *= attackMultiplier;
    m_startCombatHealthMultiplier *= healthMultiplier;
    m_startCombatStatsApplied = false;
}

void Minion::ApplyStartCombatStatMultipliers()
{
    if (m_startCombatStatsApplied)
    {
        return;
    }
    m_attack *= m_startCombatAttackMultiplier;
    m_health *= m_startCombatHealthMultiplier;
    m_startCombatStatsApplied = true;
}

int Minion::GetAttackCount() const
{
    if (m_hasMegaWindfury)
    {
        return 4;
    }

    return m_hasWindfury ? 2 : 1;
}

void Minion::ReviveWithReborn()
{
    m_health = 1;
    m_isDestroyed = false;
    m_hasReborn = false;
}

bool Minion::IsFrozen() const
{
    return m_isFrozen;
}

void Minion::SetFrozen(bool frozen)
{
    m_isFrozen = frozen;
}

void Minion::TakeDamage(Minion& source)
{
    if (HasDivineShield())
    {
        m_hasDivineShield = false;
        return;
    }

    m_health -= source.GetAttack();
    if (m_health <= 0 || (source.HasVenomous() && source.GetAttack() > 0))
    {
        // Venomous applies only after actual damage. Divine Shield returned
        // above, and a zero-attack minion cannot poison its target.
        m_isDestroyed = true;
    }
}

void Minion::SetTaunt(bool taunt)
{
    m_hasTaunt = taunt;
}

void Minion::TakeDamage(int amount)
{
    m_health -= amount;
    if (m_health <= 0)
    {
        m_isDestroyed = true;
    }
}

bool Minion::IsDestroyed() const
{
    return m_isDestroyed;
}

bool Minion::IsPlayableByCardReq(Player& player) const
{
    if (!m_card.IsPlayableByCardReq(player))
    {
        return false;
    }

    if (m_card.mustHaveToTargetToPlay && !HasAnyValidPlayTargets(player))
    {
        return false;
    }

    return true;
}

bool Minion::HasAnyValidPlayTargets(Player& player) const
{
    bool friendlyMinions = false;

    switch (m_card.targetingType)
    {
        case TargetingType::FRIENDLY_MINIONS:
            friendlyMinions = true;
            break;
        default:
            break;
    }

    if (friendlyMinions)
    {
        for (auto& minion : player.recruitField.GetAll())
        {
            if (m_card.TargetingRequirements(minion))
            {
                return true;
            }
        }
    }

    return false;
}

bool Minion::IsValidPlayTarget(Player& player, int targetIdx)
{
    if (targetIdx == -1)
    {
        if (m_card.mustHaveToTargetToPlay)
        {
            return false;
        }

        if (m_card.targetingType == TargetingType::NONE)
        {
            return true;
        }

        if (!HasAnyValidPlayTargets(player))
        {
            return true;
        }

        return false;
    }
    else
    {
        Minion& target = player.recruitField[targetIdx];

        if (!CheckTargetingType(target))
        {
            return false;
        }

        if (m_card.TargetingRequirements(target))
        {
            return true;
        }
    }

    return false;
}

bool Minion::CheckTargetingType([[maybe_unused]] Minion& target)
{
    switch (m_card.targetingType)
    {
        case TargetingType::NONE:
            return false;
        case TargetingType::FRIENDLY_MINIONS:
            return true;
        default:
            break;
    }

    return true;
}

void Minion::ActivateTrigger(TriggerType type, Minion& source)
{
    auto& trigger = m_card.power.GetTrigger();
    if (!trigger.has_value())
    {
        return;
    }

    if (trigger.value().GetTriggerType() != type)
    {
        return;
    }

    trigger.value().Run(*this, source);
}

void Minion::ActivateTask(PowerType type, Player& player)
{
    auto tasks = GetTasks(type);
    if (tasks.empty())
    {
        return;
    }

    for (auto& task : tasks)
    {
        if (player.taskStack.isStackingTasks &&
            !std::holds_alternative<SimpleTasks::RepeatNumberEndTask>(task))
        {
            player.taskStack.tasks.emplace_back(task);
        }
        else
        {
            std::visit(
                [this, &player](auto& _task) { _task.Run(player, *this); },
                task);
        }
    }
}

void Minion::ActivateTask(PowerType type, Player& player, Minion& target)
{
    auto tasks = GetTasks(type);
    if (tasks.empty())
    {
        return;
    }

    for (auto& task : tasks)
    {
        if (player.taskStack.isStackingTasks &&
            !std::holds_alternative<SimpleTasks::RepeatNumberEndTask>(task))
        {
            player.taskStack.tasks.emplace_back(task);
        }
        else
        {
            std::visit([this, &player, &target](
                           auto& _task) { _task.Run(player, *this, target); },
                       task);
        }
    }
}

void Minion::ActivateRally([[maybe_unused]] Player& player, Minion& source,
                            Minion& target)
{
    auto& trigger = m_card.power.GetTrigger();
    if (!trigger.has_value() ||
        trigger.value().GetTriggerType() != TriggerType::RALLY)
    {
        return;
    }

    trigger.value().Run(*this, source, target);
}

std::vector<TaskType> Minion::GetTasks(PowerType type)
{
    switch (type)
    {
        case PowerType::POWER:
            return m_card.power.GetBattlecryTask();
        case PowerType::DEATHRATTLE:
            return m_card.power.GetDeathrattleTask();
        case PowerType::START_OF_COMBAT:
            return m_card.power.GetStartCombatTask();
        default:
            return std::vector<TaskType>{};
    }
}
}  // namespace RosettaStone::Battlegrounds
