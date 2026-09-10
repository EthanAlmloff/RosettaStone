// Copyright (c) 2017-2024 Chris Ohk

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/CardSets/TavernSpellBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemAttackerTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ProtossBehaviorTask.hpp>
#include <effolkronium/random.hpp>

#include <utility>
#include <algorithm>
#include <map>
#include <type_traits>
#include <vector>

using Random = effolkronium::random_thread_local;

namespace RosettaStone::Battlegrounds
{
// Patient Scout progression is advanced by Player at recruit end through the
// Minion::AdvancePatientScout lifecycle hook declared on the entity.
bool Minion::IsSameInstance(const Minion& other) const noexcept
{
    if (m_index >= 0 && other.m_index >= 0)
        return m_index == other.m_index;
    if (m_poolIdx >= 0 && other.m_poolIdx >= 0)
        return m_poolIdx == other.m_poolIdx;
    return m_card.id == other.m_card.id && m_zonePos == other.m_zonePos;
}

Minion::Minion(Card card, int poolIdx)
    : m_card(std::move(card)),
      m_poolIdx(poolIdx),
      m_attack(m_card.GetAttack()),
      m_health(m_card.GetHealth()),
      m_maxHealth(m_card.GetHealth())
{
    if (GetCardID() == "BG23_009_G")
        m_spellcraftUsesRemaining = 2;
    else if (GetCardID() == "BG23_009")
        m_spellcraftUsesRemaining = 1;
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

const void* Minion::GetOwnerToken() const noexcept
{
    return m_ownerToken;
}

void Minion::SetOwnerToken(const void* token) noexcept
{
    m_ownerToken = token;
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
            if (value == 1)
            {
                if (!m_hasDivineShield)
                    m_divineShieldHitsRemaining = 0;
                m_hasDivineShield = true;
            }
            else
            {
                const bool wasDivineShielded = m_hasDivineShield;
                m_hasDivineShield = false;
                m_divineShieldHitsRemaining = 0;
                if (wasDivineShielded && getPlayerCallback)
                    getPlayerCallback().OnFriendlyDivineShieldLost(*this);
            }
            break;
        case GameTag::WINDFURY:
            m_hasWindfury = value == 1 ? true : false;
            break;
        case GameTag::MEGA_WINDFURY:
            m_hasMegaWindfury = value == 1 ? true : false;
            break;
        case GameTag::POISONOUS:
        case GameTag::VENOMOUS:
        {
            const bool hadVenomous = m_hasVenomous;
            m_hasVenomous = value == 1 ? true : false;
            if (hadVenomous && !m_hasVenomous && getPlayerCallback)
                getPlayerCallback().OnFriendlyVenomousLost(*this);
            break;
        }
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
    if (m_amalgamation && race != Race::INVALID && race != Race::ALL)
        return true;
    const auto index = static_cast<std::size_t>(race);
    if (race != Race::INVALID && index < m_extraRaces.size() && m_extraRaces[index])
        return true;
    return m_card.HasRace(race);
}

void Minion::AddRace(Race race) noexcept
{
    if (race == Race::INVALID || race == Race::ALL) return;
    const auto index = static_cast<std::size_t>(race);
    if (index < m_extraRaces.size()) m_extraRaces[index] = true;
}

bool Minion::IsMagnetic() const
{
    return m_card.gameTags.contains(GameTag::MAGNETIC) &&
           m_card.gameTags.at(GameTag::MAGNETIC) != 0;
}

void Minion::ArmMagnetization() { m_magnetizationArmed = true; }
bool Minion::ConsumeMagnetizationArm()
{
    const bool armed = m_magnetizationArmed;
    m_magnetizationArmed = false;
    return armed;
}

bool Minion::CanMagnetizeTo(const Minion& target) const
{
    if (!IsMagnetic() || target.IsDestroyed())
        return false;
    // Prosthetic Hand is the one pinned card in this supported family that
    // explicitly permits Undead in addition to Mechs.
    return target.HasRace(Race::MECHANICAL) ||
           ((GetCardID() == "BG_DEEP_015" || GetCardID() == "BG_DEEP_015_G") &&
            target.HasRace(Race::UNDEAD));
}

void Minion::MagnetizeOnto(Minion& target) const
{
    if (!CanMagnetizeTo(target))
        return;
    const bool doubled = target.ConsumeMagnetizationArm();
    const int multiplier = doubled ? 2 : 1;
    ++target.m_magnetizationCount;
    if (target.getPlayerCallback) ++target.getPlayerCallback().magnetizationsThisGame;
    target.SetAttack(target.GetAttack() + GetAttack() * multiplier);
    target.SetHealth(target.GetHealth() + GetHealth() * multiplier);
    if (HasTaunt()) target.SetTaunt(true);
    if (HasDivineShield()) target.SetGameTag(GameTag::DIVINE_SHIELD, 1);
    if (HasReborn()) target.SetReborn(true);
    if (HasWindfury()) target.SetGameTag(GameTag::WINDFURY, 1);
    if (HasVenomous()) target.SetGameTag(GameTag::POISONOUS, 1);
    if (m_card.gameTags.contains(GameTag::MEGA_WINDFURY))
        target.SetGameTag(GameTag::MEGA_WINDFURY, 1);
    if (HasDeathrattle())
    {
        for (const auto& task : m_card.power.GetDeathrattleTask())
            target.m_card.power.AddDeathrattleTask(TaskType{ task });
        target.m_hasDeathrattle = true;
    }

    // Beatboxer mirrors only a successful attachment, after the original
    // target has received its complete stat/keyword/deathrattle payload.
    // Reusing MagnetizeOnto preserves that payload and its exact ordering;
    // the owner guard prevents mirrored applications from recursing.
    if (target.getPlayerCallback && !target.getPlayerCallback().magnetizationMirrorInProgress) {
        auto& owner = target.getPlayerCallback();
        std::vector<Minion*> mirrors;
        // Combat starts from a copy of recruitField.  A start-of-combat
        // Magnetize (for example Assembler Portrait) must mirror onto that
        // combat copy, never onto the persistent recruit board.  Normal
        // recruit-phase Magnetize still resolves against recruitField.
        FieldZone* targetField = &owner.recruitField;
        owner.battleField.ForEachAlive([&](MinionData& data) {
            if (&data.value() == &target) targetField = &owner.battleField;
        });
        targetField->ForEachAlive([&](MinionData& data) {
            auto& candidate = data.value();
            if (&candidate != &target &&
                (candidate.GetCardID() == "BG26_149" ||
                 candidate.GetCardID() == "BG26_149_G"))
                mirrors.push_back(&candidate);
        });
        if (!mirrors.empty()) {
                owner.magnetizationMirrorInProgress = true;
            for (auto* mirror : mirrors) {
                const int repeats = mirror->GetCardID() == "BG26_149_G" ? 2 : 1;
                for (int i = 0; i < repeats; ++i) {
                    const auto before = mirror->GetMagnetizationCount();
                    MagnetizeOnto(*mirror);
                    // A Beatboxer mirror is itself a successful friendly
                    // Magnetize event and must advance target-local passive
                    // Trinkets independently of the original attachment.
                    if (mirror->GetMagnetizationCount() > before)
                        owner.ApplyAfterMagnetizeTrinkets(*mirror);
                }
            }
            owner.magnetizationMirrorInProgress = false;
        }
    }
}

void Minion::CopyDeathrattleTo(Minion& target) const
{
    if (!HasDeathrattle()) return;
    for (const auto& task : m_card.power.GetDeathrattleTask())
        target.m_card.power.AddDeathrattleTask(TaskType{ task });
    target.m_hasDeathrattle = true;
}

void Minion::AddDarkGiftRallyTask(TaskType&& task)
{
    m_card.power.AddRallyTask(std::move(task));
}

void Minion::AddDarkGiftDeathrattleTask(TaskType&& task)
{
    m_card.power.AddDeathrattleTask(std::move(task));
    m_hasDeathrattle = true;
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
    const int futureLobsterAttack = m_futureLobsterAttack;
    const int futureLobsterHealth = m_futureLobsterHealth;
    const int futureBallerAttack = m_futureBallerAttack;
    const int futureBallerHealth = m_futureBallerHealth;
    const int persistentMinionAttack = m_persistentMinionAttack;
    const int persistentMinionHealth = m_persistentMinionHealth;
    const int persistentTierMinionAttack = m_persistentTierMinionAttack;
    const int persistentTierMinionHealth = m_persistentTierMinionHealth;
    const auto persistentRaceAttack = m_persistentRaceAttack;
    const auto persistentRaceHealth = m_persistentRaceHealth;
    const int bloodGemCount = m_bloodGemCount;
    const int bloodGemCountThisTurn = m_bloodGemCountThisTurn;
    const int bloodGemAttack = m_bloodGemAttack;
    const int bloodGemHealth = m_bloodGemHealth;
    const int skyGolemDeathrattleCount = m_skyGolemDeathrattleCount;
    const int eternalKnightDeathCountApplied = m_eternalKnightDeathCountApplied;
    // The premium card supplies the new identity and its static keywords,
    // but conversion must not erase state accumulated by this particular
    // instance.  Dark Gifts and other recruit effects mutate these fields
    // without changing Card::gameTags, so snapshot them before replacing the
    // card metadata.
    const bool hadDeathrattle = m_hasDeathrattle;
    const bool hadTaunt = m_hasTaunt;
    const bool hadDivineShield = m_hasDivineShield;
    const int divineShieldHitsRemaining = m_divineShieldHitsRemaining;
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
    const int temporaryAttack = m_temporaryAttack;
    const int temporaryHealth = m_temporaryHealth;
    const bool temporaryTaunt = m_temporaryTaunt;
    const bool temporaryDivineShield = m_temporaryDivineShield;
    const bool temporaryReborn = m_temporaryReborn;
    const bool temporaryWindfury = m_temporaryWindfury;
    const bool temporaryMegaWindfury = m_temporaryMegaWindfury;
    const bool temporaryVenomous = m_temporaryVenomous;
    const bool temporaryStealth = m_temporaryStealth;
    m_card = std::move(premium);
    m_card.Initialize();
    m_attack = currentAttack;
    m_health = currentHealth;
    m_globalMinionAttack = globalMinionAttack;
    m_futureLobsterAttack = futureLobsterAttack;
    m_futureLobsterHealth = futureLobsterHealth;
    m_futureBallerAttack = futureBallerAttack;
    m_futureBallerHealth = futureBallerHealth;
    m_persistentMinionAttack = persistentMinionAttack;
    m_persistentMinionHealth = persistentMinionHealth;
    m_persistentTierMinionAttack = persistentTierMinionAttack;
    m_persistentTierMinionHealth = persistentTierMinionHealth;
    m_persistentRaceAttack = persistentRaceAttack;
    m_persistentRaceHealth = persistentRaceHealth;
    m_bloodGemCount = bloodGemCount;
    m_bloodGemCountThisTurn = bloodGemCountThisTurn;
    m_bloodGemAttack = bloodGemAttack;
    m_bloodGemHealth = bloodGemHealth;
    m_skyGolemDeathrattleCount = skyGolemDeathrattleCount;
    // A normal Knight that becomes golden keeps its already-realized aura;
    // only future deaths use the golden multiplier.
    m_eternalKnightDeathCountApplied = eternalKnightDeathCountApplied;
    if (m_card.id == "BG25_008_G")
        m_eternalKnightDeathCountApplied = eternalKnightDeathCountApplied;

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
    m_divineShieldHitsRemaining = divineShieldHitsRemaining;
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
    m_temporaryAttack = temporaryAttack;
    m_temporaryHealth = temporaryHealth;
    m_temporaryTaunt = temporaryTaunt;
    m_temporaryDivineShield = temporaryDivineShield;
    m_temporaryReborn = temporaryReborn;
    m_temporaryWindfury = temporaryWindfury;
    m_temporaryMegaWindfury = temporaryMegaWindfury;
    m_temporaryVenomous = temporaryVenomous;
    m_temporaryStealth = temporaryStealth;
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

bool Minion::MergeIntoGolden(const Minion& other)
{
    if (&other == this || IsGolden() || other.IsGolden() ||
        IsTemporarilyGolden() || other.IsTemporarilyGolden() ||
        !CanMakeGolden())
        return false;

    const Card thisCard = Cards::FindCardByDbfID(GetDbfID());
    const Card otherCard = Cards::FindCardByDbfID(other.GetDbfID());
    if (thisCard.normalDbfID != 0 || otherCard.normalDbfID != 0 ||
        thisCard.dbfID == 0 || otherCard.dbfID == 0 ||
        thisCard.dbfID != otherCard.dbfID ||
        thisCard.premiumDbfID == 0)
        return false;

    // Triple formation combines the copies' enchantment deltas with the
    // premium base stats.  Capture the task suffixes before MakeGolden():
    // printed tasks belong to the card definition and are supplied by the
    // premium card, while tasks appended at runtime are instance provenance
    // and must survive from both source copies.
    const auto appendDynamic = [](auto& destination, const auto& source,
                                  std::size_t baseline) {
        for (std::size_t i = baseline; i < source.size(); ++i)
            destination.emplace_back(source[i]);
    };
    const auto thisBattlecry = m_card.power.GetBattlecryTask();
    const auto thisStartCombat = m_card.power.GetStartCombatTask();
    const auto thisDeathrattle = m_card.power.GetDeathrattleTask();
    const auto thisRally = m_card.power.GetRallyTask();
    const auto otherBattlecry = other.m_card.power.GetBattlecryTask();
    const auto otherStartCombat = other.m_card.power.GetStartCombatTask();
    const auto otherDeathrattle = other.m_card.power.GetDeathrattleTask();
    const auto otherRally = other.m_card.power.GetRallyTask();
    const auto baseBattlecry = thisCard.power.GetBattlecryTask().size();
    const auto baseStartCombat = thisCard.power.GetStartCombatTask().size();
    const auto baseDeathrattle = thisCard.power.GetDeathrattleTask().size();
    const auto baseRally = thisCard.power.GetRallyTask().size();

    const int normalAttack = thisCard.GetAttack();
    const int normalHealth = thisCard.GetHealth();
    const int attackDelta = (m_attack - normalAttack) +
                            (other.m_attack - normalAttack);
    const int healthDelta = (m_health - normalHealth) +
                            (other.m_health - normalHealth);
    const int maxHealthDelta = (m_maxHealth - normalHealth) +
                               (other.m_maxHealth - normalHealth);

    // These fields are instance-owned permanent/enchantment provenance.  A
    // triple retains both copies' contributions, rather than merely keeping
    // whichever copy happened to be selected as the survivor.
    const auto sum = [](auto left, auto right) { return left + right; };
    const int globalAttack = sum(m_globalMinionAttack, other.m_globalMinionAttack);
    const int lobsterAttack = sum(m_futureLobsterAttack, other.m_futureLobsterAttack);
    const int lobsterHealth = sum(m_futureLobsterHealth, other.m_futureLobsterHealth);
    const int ballerAttack = sum(m_futureBallerAttack, other.m_futureBallerAttack);
    const int ballerHealth = sum(m_futureBallerHealth, other.m_futureBallerHealth);
    const int persistentAttack = sum(m_persistentMinionAttack, other.m_persistentMinionAttack);
    const int persistentHealth = sum(m_persistentMinionHealth, other.m_persistentMinionHealth);
    const int persistentTierAttack = sum(m_persistentTierMinionAttack, other.m_persistentTierMinionAttack);
    const int persistentTierHealth = sum(m_persistentTierMinionHealth, other.m_persistentTierMinionHealth);
    const int spellImprovement = sum(m_startCombatSpellImprovement, other.m_startCombatSpellImprovement);
    const int discoverBuffAttack = sum(m_discoverBuffAttack, other.m_discoverBuffAttack);
    const int discoverBuffHealth = sum(m_discoverBuffHealth, other.m_discoverBuffHealth);
    const int bloodGems = sum(m_bloodGemCount, other.m_bloodGemCount);
    const int bloodGemsThisTurn = sum(m_bloodGemCountThisTurn, other.m_bloodGemCountThisTurn);
    const int bloodGemAttack = sum(m_bloodGemAttack, other.m_bloodGemAttack);
    const int bloodGemHealth = sum(m_bloodGemHealth, other.m_bloodGemHealth);
    const int combatAttack = sum(m_combatPersistentAttack, other.m_combatPersistentAttack);
    const int combatHealth = sum(m_combatPersistentHealth, other.m_combatPersistentHealth);
    const auto combatKeywords = m_combatPersistentKeywords | other.m_combatPersistentKeywords;
    const int playAttack = sum(m_playCardAttackBonus, other.m_playCardAttackBonus);
    const int playHealth = sum(m_playCardHealthBonus, other.m_playCardHealthBonus);
    const int deathAttack = sum(m_deathrattleAttackTransfer, other.m_deathrattleAttackTransfer);
    const int deathHealth = sum(m_deathrattleHealthTransfer, other.m_deathrattleHealthTransfer);
    const int skyGolemDeaths = sum(m_skyGolemDeathrattleCount, other.m_skyGolemDeathrattleCount);
    const int knightDeaths = sum(m_eternalKnightDeathCountApplied, other.m_eternalKnightDeathCountApplied);
    const int knightUndeadDeaths = sum(m_eternalKnightUndeadDeathCountApplied, other.m_eternalKnightUndeadDeathCountApplied);
    const int damageDealt = sum(m_damageDealt, other.m_damageDealt);
    const int magnetizations = sum(m_magnetizationCount, other.m_magnetizationCount);
    const int temporaryAttack = sum(m_temporaryAttack, other.m_temporaryAttack);
    const int temporaryHealth = sum(m_temporaryHealth, other.m_temporaryHealth);
    const int frenzyUses = std::max(m_frenzyUses, other.m_frenzyUses);
    const int buyTriggerUses = std::max(m_buyTriggerUses, other.m_buyTriggerUses);
    const int steadyAttack = sum(m_steadyGrowthAttack, other.m_steadyGrowthAttack);
    const int steadyHealth = sum(m_steadyGrowthHealth, other.m_steadyGrowthHealth);
    const int steadySecondAttack = sum(m_steadyGrowthSecondAttack,
                                       other.m_steadyGrowthSecondAttack);
    const int steadySecondHealth = sum(m_steadyGrowthSecondHealth,
                                       other.m_steadyGrowthSecondHealth);
    const int steadyLaterAttack = sum(m_steadyGrowthLaterAttack,
                                      other.m_steadyGrowthLaterAttack);
    const int steadyLaterHealth = sum(m_steadyGrowthLaterHealth,
                                      other.m_steadyGrowthLaterHealth);
    const auto steadyTurns = std::max(m_steadyGrowthTurns,
                                      other.m_steadyGrowthTurns);
    const int avengeDeaths = sum(m_avengeDeaths, other.m_avengeDeaths);
    const int threshold = std::max(m_attackThresholdDivineShield,
                                   other.m_attackThresholdDivineShield);
    const bool thresholdTriggered = m_attackThresholdTriggered ||
                                    other.m_attackThresholdTriggered;
    const int spendThresholdCount = sum(m_spendGoldThresholdCount,
                                        other.m_spendGoldThresholdCount);
    const int darkGiftAttack = sum(m_darkGiftCounterAttack,
                                   other.m_darkGiftCounterAttack);
    const int darkGiftHealth = sum(m_darkGiftCounterHealth,
                                   other.m_darkGiftCounterHealth);
    const int incubation = std::max(m_incubationTurnsRemaining,
                                    other.m_incubationTurnsRemaining);
    const int replication = std::max(m_replicationTurnsRemaining,
                                     other.m_replicationTurnsRemaining);
    const int patientScout = std::max(m_patientScoutTurns,
                                      other.m_patientScoutTurns);
    const int spellcraftUses = std::max(m_spellcraftUsesRemaining,
                                        other.m_spellcraftUsesRemaining);
    const int felboarCounter = std::max(m_felboarSpellCounter,
                                        other.m_felboarSpellCounter);
    const int kodoUses = std::max(m_kodoSummonUses, other.m_kodoSummonUses);
    const int activateUses = std::max(m_activateUses, other.m_activateUses);
    const int startDeathrattleTriggers = std::max(
        m_startCombatDeathrattleTriggers, other.m_startCombatDeathrattleTriggers);
    const int startLeftAttackTriggers = std::max(
        m_startCombatLeftAttackTriggers, other.m_startCombatLeftAttackTriggers);
    const int eggHatch = std::max(m_eggHatchTurnsRemaining,
                                  other.m_eggHatchTurnsRemaining);
    if (m_affinityRace != Race::INVALID && other.m_affinityRace != Race::INVALID &&
        m_affinityRace != other.m_affinityRace)
        return false;
    const Race affinityRace = m_affinityRace != Race::INVALID
                                  ? m_affinityRace
                                  : other.m_affinityRace;
    const int affinityTurns = std::max(m_affinityTurns, other.m_affinityTurns);
    if (m_darkGiftCounterKind != 0 && other.m_darkGiftCounterKind != 0 &&
        m_darkGiftCounterKind != other.m_darkGiftCounterKind)
        return false;
    const int darkGiftKind = m_darkGiftCounterKind != 0
                                 ? m_darkGiftCounterKind
                                 : other.m_darkGiftCounterKind;

    if (!MakeGolden()) return false;

    m_attack = m_card.GetAttack() + attackDelta;
    m_health = m_card.GetHealth() + healthDelta;
    m_maxHealth = m_card.GetHealth() + maxHealthDelta;
    m_globalMinionAttack = globalAttack;
    m_futureLobsterAttack = lobsterAttack;
    m_futureLobsterHealth = lobsterHealth;
    m_futureBallerAttack = ballerAttack;
    m_futureBallerHealth = ballerHealth;
    m_persistentMinionAttack = persistentAttack;
    m_persistentMinionHealth = persistentHealth;
    m_persistentTierMinionAttack = persistentTierAttack;
    m_persistentTierMinionHealth = persistentTierHealth;
    for (std::size_t i = 0; i < m_persistentRaceAttack.size(); ++i) {
        m_persistentRaceAttack[i] += other.m_persistentRaceAttack[i];
        m_persistentRaceHealth[i] += other.m_persistentRaceHealth[i];
    }
    m_startCombatSpellImprovement = spellImprovement;
    m_discoverBuffAttack = discoverBuffAttack;
    m_discoverBuffHealth = discoverBuffHealth;
    m_bloodGemCount = bloodGems;
    m_bloodGemCountThisTurn = bloodGemsThisTurn;
    m_bloodGemAttack = bloodGemAttack;
    m_bloodGemHealth = bloodGemHealth;
    m_combatPersistentAttack = combatAttack;
    m_combatPersistentHealth = combatHealth;
    m_combatPersistentKeywords = combatKeywords;
    m_playCardAttackBonus = playAttack;
    m_playCardHealthBonus = playHealth;
    m_deathrattleAttackTransfer = deathAttack;
    m_deathrattleHealthTransfer = deathHealth;
    m_deathrattleStatTransferToAll = m_deathrattleStatTransferToAll || other.m_deathrattleStatTransferToAll;
    m_earthElementalDeathrattle = m_earthElementalDeathrattle || other.m_earthElementalDeathrattle;
    m_skyGolemDeathrattleCount = skyGolemDeaths;
    m_eternalKnightDeathCountApplied = knightDeaths;
    m_eternalKnightUndeadDeathCountApplied = knightUndeadDeaths;
    m_damageDealt = damageDealt;
    m_magnetizationCount = magnetizations;
    m_temporaryAttack = temporaryAttack;
    m_temporaryHealth = temporaryHealth;
    m_temporaryTaunt = m_temporaryTaunt || other.m_temporaryTaunt;
    m_temporaryDivineShield = m_temporaryDivineShield || other.m_temporaryDivineShield;
    m_temporaryReborn = m_temporaryReborn || other.m_temporaryReborn;
    m_temporaryWindfury = m_temporaryWindfury || other.m_temporaryWindfury;
    m_temporaryMegaWindfury = m_temporaryMegaWindfury || other.m_temporaryMegaWindfury;
    m_temporaryVenomous = m_temporaryVenomous || other.m_temporaryVenomous;
    m_temporaryStealth = m_temporaryStealth || other.m_temporaryStealth;
    m_frenzyUses = frenzyUses;
    m_buyTriggerUses = buyTriggerUses;
    m_steadyGrowthAttack = steadyAttack;
    m_steadyGrowthHealth = steadyHealth;
    m_steadyGrowthSecondAttack = steadySecondAttack;
    m_steadyGrowthSecondHealth = steadySecondHealth;
    m_steadyGrowthLaterAttack = steadyLaterAttack;
    m_steadyGrowthLaterHealth = steadyLaterHealth;
    m_steadyGrowthTurns = steadyTurns;
    m_avengeDeaths = avengeDeaths;
    m_attackThresholdDivineShield = threshold;
    m_attackThresholdTriggered = thresholdTriggered;
    m_endTurnBattlecryTrigger = m_endTurnBattlecryTrigger || other.m_endTurnBattlecryTrigger;
    m_spendGoldThresholdFired = m_spendGoldThresholdFired || other.m_spendGoldThresholdFired;
    m_spendGoldThresholdCount = spendThresholdCount;
    m_treasureParrotRewarded = m_treasureParrotRewarded || other.m_treasureParrotRewarded;
    m_heroDamageThresholdFired = m_heroDamageThresholdFired || other.m_heroDamageThresholdFired;
    m_darkGiftCounterAttack = darkGiftAttack;
    m_darkGiftCounterHealth = darkGiftHealth;
    m_darkGiftCounterKind = darkGiftKind;
    m_incubationTurnsRemaining = incubation;
    m_replicationTurnsRemaining = replication;
    m_patientScoutTurns = patientScout;
    m_spellcraftUsesRemaining = spellcraftUses;
    m_zestyShakerUsed = m_zestyShakerUsed || other.m_zestyShakerUsed;
    m_felboarSpellCounter = felboarCounter;
    m_kodoSummonUses = kodoUses;
    m_activateUses = activateUses;
    m_startCombatDeathrattleTriggers = static_cast<std::uint8_t>(startDeathrattleTriggers);
    m_startCombatLeftAttackTriggers = static_cast<std::uint8_t>(startLeftAttackTriggers);
    m_immuneWhileAttacking = m_immuneWhileAttacking || other.m_immuneWhileAttacking;
    m_isAttacking = m_isAttacking || other.m_isAttacking;
    m_eggHatchTurnsRemaining = eggHatch;
    m_affinityRace = affinityRace;
    m_affinityTurns = affinityTurns;
    m_polarization = m_polarization || other.m_polarization;

    // Runtime keywords and one-shot/provenance flags are retained if either
    // source carried them.  The surviving entity's zone/index/owner remain
    // authoritative, so removing the other copy cannot reorder it.
    m_hasDeathrattle = m_hasDeathrattle || other.m_hasDeathrattle;
    m_hasTaunt = m_hasTaunt || other.m_hasTaunt;
    m_hasDivineShield = m_hasDivineShield || other.m_hasDivineShield;
    m_divineShieldHitsRemaining = std::max(m_divineShieldHitsRemaining, other.m_divineShieldHitsRemaining);
    m_hasReborn = m_hasReborn || other.m_hasReborn;
    m_rebornFullHealth = m_rebornFullHealth || other.m_rebornFullHealth;
    m_hasWindfury = m_hasWindfury || other.m_hasWindfury;
    m_hasMegaWindfury = m_hasMegaWindfury || other.m_hasMegaWindfury;
    m_hasVenomous = m_hasVenomous || other.m_hasVenomous;
    m_hasStealth = m_hasStealth || other.m_hasStealth;
    m_isFrozen = m_isFrozen || other.m_isFrozen;
    m_handLocked = m_handLocked || other.m_handLocked;
    m_handLockTurns = std::max(m_handLockTurns, other.m_handLockTurns);
    m_combinedChooseOne = m_combinedChooseOne || other.m_combinedChooseOne;
    m_diesAtRecruitEnd = m_diesAtRecruitEnd || other.m_diesAtRecruitEnd;
    m_magnetizationArmed = m_magnetizationArmed || other.m_magnetizationArmed;
    m_permanentSpellcraft = m_permanentSpellcraft || other.m_permanentSpellcraft;
    m_tarecgosaBlessing = m_tarecgosaBlessing || other.m_tarecgosaBlessing;
    m_timeTurning = m_timeTurning || other.m_timeTurning;
    m_amalgamation = m_amalgamation || other.m_amalgamation;
    for (std::size_t i = 0; i < m_extraRaces.size(); ++i)
        m_extraRaces[i] = m_extraRaces[i] || other.m_extraRaces[i];
    if (m_taughtTavernSpell.empty()) m_taughtTavernSpell = other.m_taughtTavernSpell;
    if (m_lastDamageSourceIndex < 0) {
        m_lastDamageSourceIndex = other.m_lastDamageSourceIndex;
        m_lastDamageSourceCardID = other.m_lastDamageSourceCardID;
    }
    for (const auto& id : other.m_temporaryEnchantmentIDs)
    {
        // Goldrinn occurrences are transferred by Battle::CommitPersistentState
        // with their stack count; merging one deduplicated marker here would
        // lose the golden two-trigger amount.
        if (id == "BGS_018e") continue;
        if (std::find(m_temporaryEnchantmentIDs.begin(),
                      m_temporaryEnchantmentIDs.end(), id) ==
            m_temporaryEnchantmentIDs.end())
            m_temporaryEnchantmentIDs.push_back(id);
    }

    appendDynamic(m_card.power.GetBattlecryTask(), thisBattlecry, baseBattlecry);
    appendDynamic(m_card.power.GetBattlecryTask(), otherBattlecry, baseBattlecry);
    appendDynamic(m_card.power.GetStartCombatTask(), thisStartCombat, baseStartCombat);
    appendDynamic(m_card.power.GetStartCombatTask(), otherStartCombat, baseStartCombat);
    appendDynamic(m_card.power.GetDeathrattleTask(), thisDeathrattle, baseDeathrattle);
    appendDynamic(m_card.power.GetDeathrattleTask(), otherDeathrattle, baseDeathrattle);
    appendDynamic(m_card.power.GetRallyTask(), thisRally, baseRally);
    appendDynamic(m_card.power.GetRallyTask(), otherRally, baseRally);
    return true;
}

bool Minion::MakeGoldenUntilNextTurn()
{
    // A second cast in the same turn must not replace the original snapshot:
    // doing so would make expiry restore the already-premium card and leave
    // the source permanently golden.  Treat this as a non-stacking effect.
    if (IsTemporarilyGolden() || !CanMakeGolden())
        return false;
    m_temporaryGoldenOriginalCard = m_card;
    if (!MakeGolden())
    {
        m_temporaryGoldenOriginalCard.reset();
        return false;
    }
    return true;
}

int Minion::GetAttack() const
{
    return m_attack;
}

void Minion::SetAttack(int val)
{
    m_attack = val;
    if (m_attackThresholdDivineShield > 0 && !m_attackThresholdTriggered &&
        m_attack >= m_attackThresholdDivineShield)
    {
        m_attackThresholdTriggered = true;
        SetGameTag(GameTag::DIVINE_SHIELD, 1);
    }
}

void Minion::ApplyEternalKnightDeathCount(int count)
{
    if (count <= m_eternalKnightDeathCountApplied ||
        (m_card.id != "BG25_008" && m_card.id != "BG25_008_G"))
        return;
    const int multiplier = m_card.id == "BG25_008_G" ? 2 : 1;
    const int delta = count - m_eternalKnightDeathCountApplied;
    m_attack += 4 * multiplier * delta;
    m_health += 2 * multiplier * delta;
    m_eternalKnightDeathCountApplied = count;
}

void Minion::ApplyEternalKnightUndeadDeathCount(int count)
{
    if (count <= m_eternalKnightUndeadDeathCountApplied ||
        (m_card.id != "BG25_008" && m_card.id != "BG25_008_G"))
        return;
    // Eternal Portrait is an external +4/+2 aura. It applies the same
    // payload to normal and golden Eternal Knights; only the Knight's own
    // printed wherever-this-is text doubles with the golden card.
    const int multiplier = 1;
    const int delta = count - m_eternalKnightUndeadDeathCountApplied;
    m_attack += 4 * multiplier * delta;
    m_health += 2 * multiplier * delta;
    m_eternalKnightUndeadDeathCountApplied = count;
}

bool Minion::TransformTo(Card replacement)
{
    if (replacement.dbfID == 0 || replacement.GetCardType() != CardType::MINION)
        return false;
    // A transformation is a new card instance in the same entity/zone.  Build
    // a fresh instance so temporary stats, enchantment counters, keyword
    // charges, and the old card's executable powers cannot leak through the
    // identity replacement.  Preserve only engine-owned identity/zone data.
    const int index = m_index;
    const int poolIndex = m_poolIdx;
    const ZoneType zoneType = m_zoneType;
    const int zonePos = m_zonePos;
    const int lastFieldPos = m_lastFieldPos;
    auto playerCallback = std::move(getPlayerCallback);
    Minion transformed(std::move(replacement), poolIndex);
    transformed.SetIndex(index);
    transformed.SetZoneType(zoneType);
    transformed.SetZonePosition(zonePos);
    transformed.SetLastFieldPos(lastFieldPos);
    transformed.getPlayerCallback = std::move(playerCallback);
    *this = std::move(transformed);
    // Re-derive runtime keyword flags from the replacement metadata.  The
    // fresh-instance construction above already initializes these flags, but
    // keeping the reset/rebuild explicit makes the transform boundary robust
    // if construction or assignment gains additional instance state later.
    // The equivalent direct replacement operation is `m_card = std::move(replacement)`;
    // this path uses a fresh instance so no old runtime state can leak.
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
            case GameTag::DEATHRATTLE: m_hasDeathrattle = true; break;
            case GameTag::TAUNT: m_hasTaunt = true; break;
            case GameTag::DIVINE_SHIELD: m_hasDivineShield = true; break;
            case GameTag::REBORN: m_hasReborn = true; break;
            case GameTag::WINDFURY: m_hasWindfury = true; break;
            case GameTag::MEGA_WINDFURY: m_hasMegaWindfury = true; break;
            case GameTag::POISONOUS:
            case GameTag::VENOMOUS: m_hasVenomous = true; break;
            case GameTag::STEALTH: m_hasStealth = true; break;
            default: break;
        }
    }
    return true;
}

bool Minion::TransformToKeepingInstanceState(Card replacement)
{
    if (replacement.dbfID == 0 || replacement.GetCardType() != CardType::MINION)
        return false;

    // TransformTo intentionally creates a fresh instance for ordinary
    // transforms.  Keep a complete snapshot here, then install only the new
    // card identity/powers so all instance-owned enchantments and counters
    // survive this special copy operation.
    Minion preserved = *this;
    if (!TransformTo(std::move(replacement))) return false;
    Card transformedCard = std::move(m_card);
    *this = std::move(preserved);
    m_card = std::move(transformedCard);
    m_card.Initialize();
    return true;
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

void Minion::ApplyFutureLobsterStats(int attack, int health)
{
    if (attack > m_futureLobsterAttack)
    {
        m_attack += attack - m_futureLobsterAttack;
        m_futureLobsterAttack = attack;
    }
    if (health > m_futureLobsterHealth)
    {
        m_health += health - m_futureLobsterHealth;
        m_futureLobsterHealth = health;
    }
}

void Minion::ApplyFutureBallerStats(int attack, int health)
{
    if (attack > m_futureBallerAttack)
    {
        m_attack += attack - m_futureBallerAttack;
        m_futureBallerAttack = attack;
    }
    if (health > m_futureBallerHealth)
    {
        m_health += health - m_futureBallerHealth;
        m_futureBallerHealth = health;
    }
}

void Minion::ApplyPersistentMinionStats(int attack, int health)
{
    int gainedAttack = 0;
    int gainedHealth = 0;
    if (attack > m_persistentMinionAttack)
    {
        gainedAttack = attack - m_persistentMinionAttack;
        m_attack += gainedAttack;
        m_persistentMinionAttack = attack;
    }
    if (health > m_persistentMinionHealth)
    {
        gainedHealth = health - m_persistentMinionHealth;
        m_health += gainedHealth;
        m_persistentMinionHealth = health;
    }
    NotifyPersistentAttackGain(gainedAttack);
    NotifyPersistentHealthGain(gainedHealth);
}

void Minion::ApplyPersistentTierMinionStats(int tier, int attack, int health)
{
    if (GetTier() > tier) return;
    int gainedAttack = 0;
    int gainedHealth = 0;
    if (attack > m_persistentTierMinionAttack)
    {
        gainedAttack = attack - m_persistentTierMinionAttack;
        m_attack += gainedAttack;
        m_persistentTierMinionAttack = attack;
    }
    if (health > m_persistentTierMinionHealth)
    {
        gainedHealth = health - m_persistentTierMinionHealth;
        m_health += gainedHealth;
        m_persistentTierMinionHealth = health;
    }
    NotifyPersistentAttackGain(gainedAttack);
    NotifyPersistentHealthGain(gainedHealth);
}

void Minion::ApplyPersistentRaceStats(Race race, int attack, int health)
{
    const auto index = static_cast<std::size_t>(race);
    if (index >= m_persistentRaceAttack.size() || !HasRace(race)) return;
    int gainedAttack = 0;
    int gainedHealth = 0;
    if (attack > m_persistentRaceAttack[index])
    {
        gainedAttack = attack - m_persistentRaceAttack[index];
        m_attack += gainedAttack;
        m_persistentRaceAttack[index] = attack;
    }
    if (health > m_persistentRaceHealth[index])
    {
        gainedHealth = health - m_persistentRaceHealth[index];
        m_health += gainedHealth;
        m_persistentRaceHealth[index] = health;
    }
    NotifyPersistentAttackGain(gainedAttack);
    NotifyPersistentHealthGain(gainedHealth);
}

void Minion::ApplyBloodGem(int attack, int health)
{
    if (attack < 0 || health < 0)
    {
        return;
    }
    m_attack += attack;
    m_health += health;
    ++m_bloodGemCount;
    ++m_bloodGemCountThisTurn;
    m_bloodGemAttack += attack;
    m_bloodGemHealth += health;
}

std::pair<int, int> Minion::RemoveBloodGems()
{
    const auto result = std::make_pair(m_bloodGemAttack, m_bloodGemHealth);
    m_attack -= m_bloodGemAttack;
    m_health -= m_bloodGemHealth;
    m_bloodGemAttack = m_bloodGemHealth = 0;
    m_bloodGemCount = m_bloodGemCountThisTurn = 0;
    return result;
}

int Minion::GetBloodGemCount() const
{
    return m_bloodGemCount;
}

int Minion::GetBloodGemsThisTurn() const
{
    return m_bloodGemCountThisTurn;
}

int Minion::GetHealth() const
{
    return m_health;
}

int Minion::GetMaxHealth() const
{
    return m_maxHealth;
}

void Minion::SetHealth(int val)
{
    if (val > m_health)
        m_maxHealth += val - m_health;
    m_health = val;
}

void Minion::ApplyTemporaryStats(int attack, int health, bool taunt)
{
    SetAttack(GetAttack() + attack);
    SetHealth(GetHealth() + health);
    m_temporaryAttack += attack;
    m_temporaryHealth += health;
    if (taunt && !HasTaunt())
    {
        SetTaunt(true);
        m_temporaryTaunt = true;
    }
}

void Minion::ApplyCombatPersistentStats(int attack, int health)
{
    // This API represents a gain explicitly marked permanent by a combat
    // effect.  Damage and debuffs use the ordinary setters and must never be
    // recorded as persistent deltas, even if a caller passes a mixed or
    // negative value accidentally.
    attack = std::max(attack, 0);
    health = std::max(health, 0);
    if (attack == 0 && health == 0) return;
    // Tarecgosa doubles stats gained during combat, while retaining the
    // explicit persistent accounting so only those gains survive reconciliation.
    if (m_tarecgosaBlessing)
    {
        attack *= 2;
        health *= 2;
    }
    SetAttack(GetAttack() + attack);
    SetHealth(GetHealth() + health);
    m_combatPersistentAttack += attack;
    m_combatPersistentHealth += health;
    NotifyPersistentAttackGain(attack);
    NotifyPersistentHealthGain(health);
}

void Minion::NotifyPersistentAttackGain(int amount)
{
    if (amount > 0 && getPlayerCallback)
        getPlayerCallback().DispatchMinionAttackGain(*this, amount);
}

void Minion::NotifyPersistentHealthGain(int amount)
{
    if (amount > 0 && getPlayerCallback)
        getPlayerCallback().DispatchMinionHealthGain(*this, amount);
}

void Minion::ApplyCombatPersistentKeyword(GameTag tag)
{
    const auto bit = [tag]() -> std::uint32_t {
        switch (tag)
        {
            case GameTag::TAUNT: return 1u << 0;
            case GameTag::DIVINE_SHIELD: return 1u << 1;
            case GameTag::REBORN: return 1u << 2;
            case GameTag::WINDFURY: return 1u << 3;
            case GameTag::MEGA_WINDFURY: return 1u << 4;
            case GameTag::POISONOUS:
            case GameTag::VENOMOUS: return 1u << 5;
            default: return 0;
        }
    }();
    if (bit == 0 || (m_combatPersistentKeywords & bit) != 0) return;
    m_combatPersistentKeywords |= bit;
    SetGameTag(tag, 1);
}

void Minion::ReconcileCombatPersistentState(const Minion& combatCopy)
{
    // The copy starts with this entity's counters.  Only increments made via
    // the explicit persistent API are committed; combat damage, start-combat
    // multipliers, temporary effects, and summoned entities never leak.
    const int attack = combatCopy.m_combatPersistentAttack -
                       m_combatPersistentAttack;
    const int health = combatCopy.m_combatPersistentHealth -
                       m_combatPersistentHealth;
    if (attack != 0 || health != 0)
    {
        SetAttack(GetAttack() + attack);
        SetHealth(GetHealth() + health);
        m_combatPersistentAttack += attack;
        m_combatPersistentHealth += health;
    }
    const auto newlyPersistent = combatCopy.m_combatPersistentKeywords &
                                 ~m_combatPersistentKeywords;
    constexpr std::array<std::pair<std::uint32_t, GameTag>, 6> tags{{
        {1u << 0, GameTag::TAUNT}, {1u << 1, GameTag::DIVINE_SHIELD},
        {1u << 2, GameTag::REBORN}, {1u << 3, GameTag::WINDFURY},
        {1u << 4, GameTag::MEGA_WINDFURY}, {1u << 5, GameTag::POISONOUS}}};
    for (const auto& [bit, tag] : tags)
    {
        if ((newlyPersistent & bit) != 0)
            ApplyCombatPersistentKeyword(tag);
    }
}

void Minion::BeginPoetCombatSnapshot(bool eligible, int multiplier) noexcept
{
    m_poetCombatEligible = eligible;
    m_poetCombatMultiplier = multiplier;
    m_poetCombatAttack = GetAttack();
    m_poetCombatHealth = GetHealth();
    m_poetCombatKeywords = 0;
    if (HasTaunt()) m_poetCombatKeywords |= 1u << 0;
    if (HasDivineShield()) m_poetCombatKeywords |= 1u << 1;
    if (HasReborn()) m_poetCombatKeywords |= 1u << 2;
    if (HasWindfury()) m_poetCombatKeywords |= 1u << 3;
    if (HasMegaWindfury()) m_poetCombatKeywords |= 1u << 4;
    if (HasVenomous()) m_poetCombatKeywords |= 1u << 5;
}

void Minion::ApplyTemporaryKeyword(GameTag tag)
{
    switch (tag)
    {
        case GameTag::TAUNT:
            if (!HasTaunt()) { SetTaunt(true); m_temporaryTaunt = true; }
            break;
        case GameTag::DIVINE_SHIELD:
            if (!HasDivineShield()) { SetGameTag(tag, 1); m_temporaryDivineShield = true; }
            break;
        case GameTag::REBORN:
            if (!HasReborn()) { SetReborn(true); m_temporaryReborn = true; }
            break;
        case GameTag::WINDFURY:
            if (!HasWindfury()) { SetGameTag(tag, 1); m_temporaryWindfury = true; }
            break;
        case GameTag::MEGA_WINDFURY:
            if (!HasWindfury()) { SetGameTag(tag, 1); m_temporaryMegaWindfury = true; }
            break;
        case GameTag::POISONOUS:
        case GameTag::VENOMOUS:
            if (!HasVenomous()) { SetGameTag(GameTag::POISONOUS, 1); m_temporaryVenomous = true; }
            break;
        case GameTag::STEALTH:
            if (!m_hasStealth) { SetGameTag(GameTag::STEALTH, 1); m_temporaryStealth = true; }
            break;
        default:
            break;
    }
}

void Minion::ApplyTemporaryEnchantment(TemporaryEnchantment kind, int attack,
                                       int health)
{
    switch (kind)
    {
        case TemporaryEnchantment::Stats:
            ApplyTemporaryStats(attack, health);
            break;
        case TemporaryEnchantment::StatsAndTaunt:
            ApplyTemporaryStats(attack, health, true);
            break;
        case TemporaryEnchantment::StatsAndWindfury:
            ApplyTemporaryStats(attack, health);
            ApplyTemporaryKeyword(GameTag::WINDFURY);
            break;
        case TemporaryEnchantment::StatsAndReborn:
            ApplyTemporaryStats(attack, health);
            ApplyTemporaryKeyword(GameTag::REBORN);
            break;
        case TemporaryEnchantment::DivineShield:
            ApplyTemporaryKeyword(GameTag::DIVINE_SHIELD);
            break;
        case TemporaryEnchantment::Venomous:
            ApplyTemporaryKeyword(GameTag::VENOMOUS);
            break;
        case TemporaryEnchantment::StatsAndStealth:
            ApplyTemporaryStats(attack, health);
            ApplyTemporaryKeyword(GameTag::STEALTH);
            break;
    }
}

void Minion::RecordTemporaryEnchantment(std::string_view enchantmentID)
{
    if (enchantmentID.empty()) return;
    const auto found = std::find(m_temporaryEnchantmentIDs.begin(),
                                 m_temporaryEnchantmentIDs.end(),
                                 enchantmentID);
    if (found == m_temporaryEnchantmentIDs.end())
        m_temporaryEnchantmentIDs.emplace_back(enchantmentID);
}

void Minion::RecordTemporaryEnchantmentOccurrence(std::string_view enchantmentID)
{
    if (!enchantmentID.empty())
        m_temporaryEnchantmentIDs.emplace_back(enchantmentID);
}

bool Minion::HasTemporaryEnchantment(std::string_view enchantmentID) const
{
    return std::find(m_temporaryEnchantmentIDs.begin(),
                     m_temporaryEnchantmentIDs.end(), enchantmentID) !=
           m_temporaryEnchantmentIDs.end();
}

void Minion::ExpireTemporaryEffects()
{
    SetAttack(GetAttack() - m_temporaryAttack);
    SetHealth(GetHealth() - m_temporaryHealth);
    if (m_temporaryTaunt)
        SetTaunt(false);
    if (m_temporaryDivineShield)
        SetGameTag(GameTag::DIVINE_SHIELD, 0);
    if (m_temporaryReborn)
        SetReborn(false);
    if (m_temporaryWindfury)
        SetGameTag(GameTag::WINDFURY, 0);
    if (m_temporaryMegaWindfury)
        SetGameTag(GameTag::MEGA_WINDFURY, 0);
    if (m_temporaryVenomous)
        SetGameTag(GameTag::POISONOUS, 0);
    if (m_temporaryStealth)
        SetGameTag(GameTag::STEALTH, 0);
    m_temporaryAttack = 0;
    m_temporaryHealth = 0;
    m_temporaryTaunt = false;
    m_temporaryDivineShield = false;
    m_temporaryReborn = false;
    m_temporaryWindfury = false;
    m_temporaryMegaWindfury = false;
    m_temporaryVenomous = false;
    m_temporaryStealth = false;
    m_temporaryEnchantmentIDs.clear();

    // Temporary Golden conversions change card identity (and therefore
    // golden-only powers) but must not erase stats, buffs, zone identity, or
    // runtime keyword state accumulated while the conversion was active.
    if (m_temporaryGoldenOriginalCard.has_value())
    {
        m_card = std::move(*m_temporaryGoldenOriginalCard);
        m_card.Initialize();
        m_temporaryGoldenOriginalCard.reset();
    }
}

bool Minion::IsLavaLurker() const noexcept
{
    return GetCardID() == "BG23_009" || GetCardID() == "BG23_009_G";
}

void Minion::ResetSpellcraftUses() noexcept
{
    m_spellcraftUsesRemaining =
        GetCardID() == "BG23_009_G" ? 2 : (GetCardID() == "BG23_009" ? 1 : 0);
}

bool Minion::ConsumeSpellcraftUse() noexcept
{
    if (m_spellcraftUsesRemaining <= 0) return false;
    --m_spellcraftUsesRemaining;
    return true;
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

void Minion::SetDivineShieldHits(int hits)
{
    m_hasDivineShield = hits > 0;
    m_divineShieldHitsRemaining = std::max(0, hits - 1);
}

int Minion::DivineShieldHitsRemaining() const
{
    return m_hasDivineShield ? m_divineShieldHitsRemaining + 1 : 0;
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

void Minion::SetStartCombatDeathrattleTrigger(bool enabled)
{
    if (!enabled) {
        m_startCombatDeathrattleTriggers = 0;
        return;
    }
    if (m_startCombatDeathrattleTriggers < 255)
        ++m_startCombatDeathrattleTriggers;
}

bool Minion::HasStartCombatDeathrattleTrigger() const
{
    return m_startCombatDeathrattleTriggers != 0;
}

bool Minion::ConsumeStartCombatDeathrattleTrigger()
{
    if (m_startCombatDeathrattleTriggers == 0) return false;
    --m_startCombatDeathrattleTriggers;
    return true;
}

void Minion::SetStartCombatLeftAttack(bool enabled)
{
    if (!enabled) {
        m_startCombatLeftAttackTriggers = 0;
        return;
    }
    if (m_startCombatLeftAttackTriggers < 255)
        ++m_startCombatLeftAttackTriggers;
}

bool Minion::HasStartCombatLeftAttack() const
{
    return m_startCombatLeftAttackTriggers != 0;
}

bool Minion::ConsumeStartCombatLeftAttack()
{
    if (m_startCombatLeftAttackTriggers == 0) return false;
    --m_startCombatLeftAttackTriggers;
    return true;
}

void Minion::ApplyStartCombatLeftAttack(const Minion& left)
{
    if (!ConsumeStartCombatLeftAttack())
        return;
    m_attack += left.GetAttack();
}

void Minion::SetImmuneWhileAttacking(bool enabled)
{
    m_immuneWhileAttacking = enabled;
}

bool Minion::HasImmuneWhileAttacking() const
{
    return m_immuneWhileAttacking;
}

void Minion::SetAttacking(bool attacking)
{
    m_isAttacking = attacking;
}

bool Minion::IsAttacking() const
{
    return m_isAttacking;
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
    m_health = m_rebornFullHealth ? m_maxHealth : 1;
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
    if (m_immuneWhileAttacking && m_isAttacking)
        return;
    if (HasDivineShield())
    {
        if (m_divineShieldHitsRemaining > 0)
        {
            --m_divineShieldHitsRemaining;
            if (m_divineShieldHitsRemaining == 0)
                m_hasDivineShield = false;
        }
        else
        {
            m_hasDivineShield = false;
        }
        if (!m_hasDivineShield && getPlayerCallback)
        {
            getPlayerCallback().OnFriendlyDivineShieldLost(*this);
            getPlayerCallback().OnFriendlyMinionDamaged(*this);
        }
        return;
    }

    SetLastDamageSource(source);
    const int damage = source.GetAttack();
    m_health -= damage;
    if (source.getPlayerCallback)
        source.getPlayerCallback().RecordTreasureParrotDamage(source, damage);
    if (m_health <= 0 || (source.HasVenomous() && damage > 0))
    {
        // Venomous applies only after actual damage. Divine Shield returned
        // above, and a zero-attack minion cannot poison its target.
        m_isDestroyed = true;
    }
    const bool limitedFrenzy = GetCardID() == "BG20_204" || GetCardID() == "BG20_204_G";
    const int frenzyLimit = GetCardID() == "BG20_204_G" ? 2 : 1;
    if (damage > 0 && !m_isDestroyed &&
        (!limitedFrenzy || m_frenzyUses < frenzyLimit) && getPlayerCallback)
    {
        ++m_frenzyUses;
        ActivateTrigger(TriggerType::TAKE_DAMAGE, *this);
    }
    if (damage > 0 && getPlayerCallback)
        getPlayerCallback().OnFriendlyMinionDamaged(*this);
}

void Minion::SetTaunt(bool taunt)
{
    m_hasTaunt = taunt;
}

void Minion::TakeDamage(int amount)
{
    if (m_immuneWhileAttacking && m_isAttacking)
        return;
    m_health -= amount;
    if (m_health <= 0)
    {
        m_isDestroyed = true;
    }
    const bool limitedFrenzy = GetCardID() == "BG20_204" || GetCardID() == "BG20_204_G";
    const int frenzyLimit = GetCardID() == "BG20_204_G" ? 2 : 1;
    if (amount > 0 && !m_isDestroyed && (!limitedFrenzy || m_frenzyUses < frenzyLimit) && getPlayerCallback)
    {
        ++m_frenzyUses;
        ActivateTrigger(TriggerType::TAKE_DAMAGE, *this);
    }
    if (amount > 0 && getPlayerCallback)
        getPlayerCallback().OnFriendlyMinionDamaged(*this);
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
    if (m_card.targetingType == TargetingType::TAVERN_MINIONS)
    {
        for (auto& minion : player.tavern.fieldZone.GetAll())
        {
            if (!minion.get().IsDestroyed() && !minion.get().GetCardID().empty() &&
                m_card.TargetingRequirements(minion.get()))
                return true;
        }
        return false;
    }

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
    else if (m_card.targetingType == TargetingType::TAVERN_MINIONS)
    {
        if (targetIdx < 0 || targetIdx >= player.tavern.fieldZone.GetCount())
            return false;
        Minion& target = player.tavern.fieldZone[static_cast<std::size_t>(targetIdx)];
        return !target.IsDestroyed() && !target.GetCardID().empty() &&
               m_card.TargetingRequirements(target);
    }
    else
    {
        Minion& target = player.recruitField[targetIdx];

        if (m_card.playRequirements.contains(PlayReq::REQ_NONSELF_TARGET) &&
            IsSameInstance(target))
            return false;

        // Mind Muck's targeting metadata is a friendly-minion arrow, but its
        // semantic requirement is a friendly Demon.  Keep legality aligned
        // with ConsumeRandomTavernTask so an invalid target cannot consume
        // the Tavern and then fail the battlecry.
        if ((GetCardID() == "BG23_357" || GetCardID() == "BG23_357_G") &&
            !target.HasRace(Race::DEMON))
            return false;

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
    // Baby Y'Shaarj observes every friendly summon, but only summons whose
    // card tier matches the owner's current Tavern tier. The printed effect
    // buffs *that summoned entity*, not every minion in the warband. Resolve
    // against the authoritative owner callback and let each Buddy copy stack.
    if (type == TriggerType::SUMMON && source.GetTier() == getPlayerCallback().currentTier)
    {
        auto& owner = getPlayerCallback();
        int amount = 0;
        auto& field = owner.isInCombat ? owner.battleField : owner.recruitField;
        field.ForEachAlive([&amount](MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "TB_BaconShop_HERO_92_Buddy") amount += 4;
            else if (id == "TB_BaconShop_HERO_92_Buddy_G") amount += 8;
        });
        if (amount != 0)
            source.ApplyPersistentMinionStats(amount, amount);
    }
    if (type == TriggerType::SUMMON &&
        (source.GetCardID() == "BG22_HERO_305t" || source.GetCardID() == "BG22_HERO_305t_G")) {
        const int amount = GetCardID() == "BG22_HERO_305_Buddy" ? 2
                         : GetCardID() == "BG22_HERO_305_Buddy_G" ? 4 : 0;
        if (amount > 0) ApplyPersistentMinionStats(amount, amount);
    }
    auto& trigger = m_card.power.GetTrigger();
    if (!trigger.has_value())
    {
        return;
    }

    if (trigger.value().GetTriggerType() != type)
    {
        return;
    }

    if (type == TriggerType::BUY_MINION || type == TriggerType::AFTER_CAST_SPELL)
        trigger.value().Run(*this, source, source);
    else
        trigger.value().Run(*this, source);
}

void Minion::ActivateTask(PowerType type, Player& player)
{
    // Jr. Navigator's Battlecry discounts Lead Explorer for future uses.
    // Store the discount as a negative cost delta; this path is reached once
    // for each actual Battlecry replay, including Brann-style repeats.
    if (type == PowerType::POWER &&
        (GetCardID() == "TB_BaconShop_HERO_42_Buddy" ||
         GetCardID() == "TB_BaconShop_HERO_42_Buddy_G"))
    {
        // Lead Explorer's discount is player-owned hero-power state.  Keep
        // this on the Batch1 state used by EffectiveHeroPowerCost so every
        // replay (including Brann-style Battlecry repeats) is applied to the
        // same cost path and persists across recruit turns.
        player.season14.heroPowerBatch1.leadExplorerCostDelta -=
            GetCardID().ends_with("_G") ? 4 : 2;
        player.season14.RecordBattlecry();
        player.AdvanceDarkGiftCounters(1);
        return;
    }
    if (type == PowerType::POWER && !TaughtTavernSpell().empty()) {
        const auto spell = Cards::FindCardByID(TaughtTavernSpell());
        const auto behavior = FindTavernSpellBehavior(TaughtTavernSpell());
        // Generated taught spells are resolved only through the canonical
        // Tavern resolver. Unknown/unimplemented identities fail closed.
        if (spell.id.empty() || behavior.effect == TavernSpellEffect::NONE)
            return;
        // Magicfin Apprentice's Battlecry is represented by the taught-spell
        // payload rather than a normal POWER task.  Keep the lifetime
        // Battlecry counter (and Dark Gift Battlecry counter) aligned with
        // ordinary Battlecry dispatches before resolving that payload.
        player.season14.RecordBattlecry();
        player.AdvanceDarkGiftCounters(1);
        player.season14.pendingTaughtSpell =
            {true, static_cast<std::uint64_t>(GetIndex()), spell.dbfID};
        (void)player.CastTavernSpellFree(TaughtTavernSpell(), 1);
        if (player.season14.pendingDecision != Season14Decision::CHOOSE_ONE)
            player.season14.pendingTaughtSpell = {};
        return;
    }
    auto tasks = GetTasks(type);
    if (tasks.empty())
    {
        return;
    }
    if (type == PowerType::POWER)
    {
        player.season14.RecordBattlecry();
        player.AdvanceDarkGiftCounters(1);
    }

    int repeats = 1;
    if (type == PowerType::DEATHRATTLE) {
        // During combat battleField is the copied authoritative board;
        // recruitField still contains the originals.  Outside combat use the
        // recruit board.  Sum every Titus copy: the effect stacks rather than
        // taking the strongest copy, and Titus has no deathrattle of its own.
        auto& field = player.isInCombat ? player.battleField : player.recruitField;
        field.ForEachAlive([&repeats](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG25_354") ++repeats;
            else if (id == "BG25_354_G") repeats += 2;
        });
    }
    if (type == PowerType::POWER)
    {
        // Dragon Skull reacts to each resolved Battlecry, including every
        // repetition introduced by Battlecry-doubling effects.  Resolve the
        // edge targets after repeat multiplicity is known, but before the
        // task body, so each repeated Battlecry sees the authoritative board
        // edges and receives exactly one +A/+H application.
        auto& battlecryField = player.isInCombat ? player.battleField : player.recruitField;
        for (int repeat = 0; repeat < repeats; ++repeat)
        {
            for (const auto& trinket : player.season14.trinkets)
            {
                if (!trinket.active || trinket.remainingUses == 0) continue;
                const auto behavior = FindTrinketBehavior(
                    Cards::FindCardByDbfID(trinket.dbfID).id);
                if (behavior.effect != TrinketEffect::BATTLECRY_EDGE_STATS) continue;
                std::vector<Minion*> edges;
                battlecryField.ForEachAlive([&edges](MinionData& data) {
                    edges.push_back(&data.value());
                });
                if (!edges.empty()) {
                    edges.front()->SetAttack(edges.front()->GetAttack() + behavior.attack);
                    edges.front()->SetHealth(edges.front()->GetHealth() + behavior.health);
                    if (edges.size() > 1) {
                        edges.back()->SetAttack(edges.back()->GetAttack() + behavior.attack);
                        edges.back()->SetHealth(edges.back()->GetHealth() + behavior.health);
                    }
                }
            }
        }
    }
    for (auto& task : tasks)
    {
        if (player.taskStack.isStackingTasks &&
            !std::holds_alternative<SimpleTasks::RepeatNumberEndTask>(task))
        {
            // Deferred execution must preserve Titus's multiplicity too;
            // enqueue one task instance per repetition rather than silently
            // collapsing stacked Deathrattles back to one.
            for (int repeat = 0; repeat < repeats; ++repeat)
                player.taskStack.tasks.emplace_back(task);
        }
        else
        {
            for (int repeat = 0; repeat < repeats; ++repeat)
                std::visit(
                    [this, &player](auto& _task) { _task.Run(player, *this); },
                    task);
        }
    }
}

void Minion::ActivateTask(PowerType type, Player& player, Minion& target)
{
    auto tasks = GetTasks(type);
    ActivateTask(type, player, target, tasks);
}

void Minion::ActivateTask(PowerType type, Player& player, Minion& target,
                          const std::vector<TaskType>& tasks)
{
    // Weebomination's printed effect is an end-of-turn aura, not a
    // Battlecry.  Keep the calculation at its targeted dispatch boundary;
    // this path must not advance the lifetime Battlecry counter used by
    // Murky Sticker.
    if (type == PowerType::POWER &&
        (GetCardID() == "TB_BaconShop_HERO_34_Buddy" ||
         GetCardID() == "TB_BaconShop_HERO_34_Buddy_G"))
    {
        if (&target == this || target.IsDestroyed()) return;
        const int maxHealth = player.season14.heroPowerBatch1.StartingHealth(
            player.hero.card.GetHealth());
        const int missingHealth = std::max(0, maxHealth - player.hero.health);
        const int multiplier = GetCardID().ends_with("_G") ? 2 : 1;
        target.SetHealth(target.GetHealth() + multiplier * missingHealth);
        return;
    }
    if (tasks.empty())
    {
        return;
    }
    // Task::Run is intentionally non-const, while this overload accepts a
    // shared task list.  Execute mutable copies so dispatch remains valid for
    // stateful task variants without mutating the source definition.
    auto taskCopies = tasks;
    if (type == PowerType::POWER)
    {
        player.season14.RecordBattlecry();
        player.AdvanceDarkGiftCounters(1);
    }

    int repeats = 1;
    if (type == PowerType::DEATHRATTLE) {
        auto& field = player.isInCombat ? player.battleField : player.recruitField;
        field.ForEachAlive([&repeats](const MinionData& data) {
            const auto& id = data.value().GetCardID();
            if (id == "BG25_354") ++repeats;
            else if (id == "BG25_354_G") repeats += 2;
        });
    }
    for (auto& task : taskCopies)
    {
        if (player.taskStack.isStackingTasks &&
            !std::holds_alternative<SimpleTasks::RepeatNumberEndTask>(task))
        {
            for (int repeat = 0; repeat < repeats; ++repeat)
                player.taskStack.tasks.emplace_back(task);
        }
        else
        {
            for (int repeat = 0; repeat < repeats; ++repeat)
                std::visit([this, &player, &target](
                               auto& _task) { _task.Run(player, *this, target); },
                           task);
        }
    }
}

void Minion::SetLastDamageSource(const Minion& source) noexcept
{
    m_lastDamageSourceIndex = source.GetIndex();
    m_lastDamageSourceCardID = std::string(source.GetCardID());
}

void Minion::SetPlayCardStatBonus(int attack, int health)
{
    // Multiple persistent play-card gifts stack on the same minion.
    m_playCardAttackBonus += attack;
    m_playCardHealthBonus += health;
}

void Minion::ApplyPlayCardStatBonus()
{
    SetAttack(GetAttack() + m_playCardAttackBonus);
    SetHealth(GetHealth() + m_playCardHealthBonus);
}

void Minion::SetAttackThresholdDivineShield(int threshold)
{
    m_attackThresholdDivineShield = threshold;
    m_attackThresholdTriggered = false;
    // A threshold aura can be attached after a stat buff (for example when
    // the minion is created above the threshold).  Resolve that state
    // immediately; waiting for a later SetAttack would make the result
    // depend on mutation order.
    if (threshold > 0 && m_attack >= threshold)
    {
        m_attackThresholdTriggered = true;
        SetGameTag(GameTag::DIVINE_SHIELD, 1);
    }
}

void Minion::SetEndTurnBattlecryTrigger(bool enabled)
{
    m_endTurnBattlecryTrigger = enabled;
}

bool Minion::HasEndTurnBattlecryTrigger() const
{
    return m_endTurnBattlecryTrigger;
}

bool Minion::HasBattlecry() const
{
    return !m_card.power.GetBattlecryTask().empty();
}

void Minion::SetDeathrattleStatTransfer(int attack, int health)
{
    m_deathrattleAttackTransfer = attack;
    m_deathrattleHealthTransfer = health;
}

void Minion::SetDeathrattleStatTransferToAll(bool enabled)
{
    m_deathrattleStatTransferToAll = enabled;
}

bool Minion::DeathrattleStatTransferToAll() const
{
    return m_deathrattleStatTransferToAll;
}

void Minion::SetEarthElementalDeathrattle(bool enabled)
{
    m_earthElementalDeathrattle = enabled;
}

bool Minion::HasEarthElementalDeathrattle() const
{
    return m_earthElementalDeathrattle;
}

void Minion::SetPowderKegDeathrattleAttack(int attack)
{
    m_powderKegDeathrattleAttack = std::max(0, attack);
}

int Minion::PowderKegDeathrattleAttack() const noexcept
{
    return m_powderKegDeathrattleAttack;
}

void Minion::ApplySkyGolemDeathrattleCount(int count)
{
    if (count <= m_skyGolemDeathrattleCount) return;
    const int delta = count - m_skyGolemDeathrattleCount;
    const int scale = IsGolden() ? 8 : 4;
    const int healthScale = IsGolden() ? 4 : 2;
    m_attack += delta * scale;
    m_health += delta * healthScale;
    m_skyGolemDeathrattleCount = count;
}

int Minion::DeathrattleAttackTransfer() const
{
    return m_deathrattleAttackTransfer;
}

int Minion::DeathrattleHealthTransfer() const
{
    return m_deathrattleHealthTransfer;
}

void Minion::SetDarkGiftCounter(int attack, int health, int kind,
                                int currentCount)
{
    if (m_darkGiftCounterKind == kind && kind != 0)
    {
        m_darkGiftCounterAttack += attack;
        m_darkGiftCounterHealth += health;
        m_attack += attack * currentCount;
        m_health += health * currentCount;
        return;
    }
    m_darkGiftCounterAttack = attack;
    m_darkGiftCounterHealth = health;
    m_darkGiftCounterKind = kind;
    if (currentCount > 0)
    {
        m_attack += attack * currentCount;
        m_health += health * currentCount;
    }
}

void Minion::ApplyDarkGiftCounterStep(int kind)
{
    if (m_darkGiftCounterKind != kind) return;
    m_attack += m_darkGiftCounterAttack;
    m_health += m_darkGiftCounterHealth;
}

void Minion::SetIncubation(int turns)
{
    if (turns > 0)
    {
        m_incubationTurnsRemaining = turns;
    }
}

void Minion::AdvanceIncubation()
{
    if (m_incubationTurnsRemaining <= 0)
    {
        return;
    }
    --m_incubationTurnsRemaining;
    if (m_incubationTurnsRemaining == 0)
    {
        m_attack *= 2;
        m_health *= 2;
    }
}

int Minion::IncubationTurnsRemaining() const
{
    return m_incubationTurnsRemaining;
}

bool Minion::AdvanceEggHatch() noexcept
{
    if (m_eggHatchTurnsRemaining <= 0) return false;
    return --m_eggHatchTurnsRemaining == 0;
}

void Minion::SetReplication(int turns)
{
    if (turns > 0) m_replicationTurnsRemaining = turns;
}

bool Minion::AdvanceReplication()
{
    if (m_replicationTurnsRemaining <= 0) return false;
    --m_replicationTurnsRemaining;
    if (m_replicationTurnsRemaining != 0) return false;
    m_replicationTurnsRemaining = 2;
    return true;
}

void Minion::ActivateHeroDamageTrigger()
{
    auto& trigger = m_card.power.GetTrigger();
    if (!trigger.has_value() ||
        trigger.value().GetTriggerType() != TriggerType::HERO_DAMAGE)
    {
        return;
    }

    // The observer is both owner and SELF event source.  This keeps a
    // hero-damage trigger from firing once for every sibling on the board.
    trigger.value().Run(*this, *this);
}

void Minion::ResetFrenzyUses()
{
    m_frenzyUses = 0;
}

void Minion::ActivateRally([[maybe_unused]] Player& player, Minion& source,
                            Minion& target)
{
    auto& trigger = m_card.power.GetTrigger();
    if (trigger.has_value() &&
        trigger.value().GetTriggerType() == TriggerType::RALLY)
    {
        trigger.value().Run(*this, source, target);
    }
    for (auto& task : m_card.power.GetRallyTask())
    {
        std::visit(
            [this, &player, &source, &target](auto& rallyTask) {
                // Most Rally tasks use the observer (the minion carrying the
                // Rally text) as their source so "other minions" semantics
                // remain relative to that observer.  Roaring Recruiter's
                // attacking-minion task is different: its source and target
                // are the attacker declared by Battle::Attack, not the
                // defender and not the observing recruiter.
                if constexpr (std::is_same_v<
                                  std::decay_t<decltype(rallyTask)>,
                                  SimpleTasks::AttackingMinionBuffTask>)
                {
                    const bool isAttacker =
                        &source == this ||
                        (source.GetIndex() >= 0 &&
                         source.GetIndex() == this->GetIndex());
                    if (!isAttacker)
                    {
                        rallyTask.Run(player, source, source);
                    }
                }
                else if constexpr (std::is_same_v<
                                       std::decay_t<decltype(rallyTask)>,
                                       SimpleTasks::RallyBloodGemAttackerTask>)
                {
                    const bool isAttacker =
                        &source == this ||
                        (source.GetIndex() >= 0 &&
                         source.GetIndex() == this->GetIndex());
                    if (!isAttacker)
                        rallyTask.Run(player, source, source);
                }
                else if constexpr (std::is_same_v<
                                       std::decay_t<decltype(rallyTask)>,
                                       SimpleTasks::RallyRaceBuffTask>)
                {
                    // Race-gated attack triggers inspect the attacker, not
                    // the observing minion.  Passing *this here made
                    // Cage Gnawer fire for every attack simply because the
                    // observer itself is a Beast.
                    rallyTask.Run(player, source, target);
                }
                else
                {
                    rallyTask.Run(player, *this, target);
                }
            },
            task);
    }
    // Rallying Cry duplicates every Rally dispatch, not only the synthetic
    // end-turn Rally emitted by a Trinket.  Guard the nested call so a
    // duplicated resolution cannot recurse indefinitely.
    if (player.season14.HasGeneratedRewardRallyingCry() &&
        !player.season14.generatedRewardRallyingCryResolving)
    {
        player.season14.generatedRewardRallyingCryResolving = true;
        ActivateRally(player, source, target);
        player.season14.generatedRewardRallyingCryResolving = false;
    }
}

bool Minion::CanActivate(const Player& player, int targetIdx) const
{
    const auto& definition = m_card.power.GetActivate();
    if (!definition.has_value() || definition->effect == ActivateEffect::NONE ||
        m_activateUses <= 0 || player.remainCoin < definition->cost)
    {
        return false;
    }
    if (definition->effect == ActivateEffect::BUFF_TARGET ||
        definition->effect == ActivateEffect::SET_TARGET_STATS ||
        definition->effect == ActivateEffect::TRIGGER_RALLY ||
        definition->effect == ActivateEffect::TRIGGER_BATTLECRY)
    {
        if (targetIdx < 0 || targetIdx >= player.recruitField.GetCount() ||
            targetIdx == GetZonePosition() ||
            player.recruitField[static_cast<std::size_t>(targetIdx)].IsDestroyed())
            return false;
        if (definition->effect == ActivateEffect::TRIGGER_BATTLECRY &&
            (!player.recruitField[static_cast<std::size_t>(targetIdx)].HasBattlecry() ||
             player.recruitField[static_cast<std::size_t>(targetIdx)].RequiresPlayTarget() ||
             player.recruitField[static_cast<std::size_t>(targetIdx)]
                     .GetTasks(PowerType::POWER)
                     .empty()))
            return false;
            return true;
    }
    if (definition->effect == ActivateEffect::APPLY_REBORN)
        return targetIdx >= 0 && targetIdx < player.recruitField.GetCount() &&
               targetIdx != GetZonePosition() &&
               !player.recruitField[static_cast<std::size_t>(targetIdx)].IsDestroyed();
    if (definition->effect == ActivateEffect::DESTROY_UNDEAD_BUFF_SELF)
    {
        if (targetIdx >= 0) return false;
        bool hasTarget = false;
        player.recruitField.ForEachAlive([&](const MinionData& data) {
            if (&data.value() != this && data.value().HasRace(Race::UNDEAD))
                hasTarget = true;
        });
        return hasTarget;
    }
    if (definition->effect == ActivateEffect::ADD_CARD)
    {
        if (player.hand.IsFull() || definition->amount <= 0 ||
            definition->cardID.empty())
            return false;
        const auto card = Cards::FindCardByID(definition->cardID);
        if (card.id.empty() ||
            (card.GetCardType() != CardType::SPELL &&
             card.GetCardType() != CardType::BATTLEGROUND_SPELL))
            return false;
    }
    if (definition->effect == ActivateEffect::RANDOM_CARD ||
        definition->effect == ActivateEffect::RANDOM_CHROMADRAKE)
        return !player.hand.IsFull() && targetIdx < 0;
    if (definition->effect == ActivateEffect::TAKE_HIGHEST_TAVERN)
    {
        if (player.hand.IsFull() || targetIdx >= 0)
            return false;
        bool hasCandidate = false;
        player.tavern.fieldZone.ForEachAlive(
            [&hasCandidate](const MinionData&) { hasCandidate = true; });
        return hasCandidate;
    }
    if (definition->effect == ActivateEffect::ACTIVATE_FISHBAIT)
        return targetIdx >= 0 &&
               targetIdx < player.tavern.fieldZone.GetCount() &&
               !player.tavern.fieldZone[static_cast<std::size_t>(targetIdx)]
                    .IsDestroyed();
    if (definition->effect == ActivateEffect::DEVOUR_TAVERN_DEMONS)
    {
        if (targetIdx >= 0 || player.tavern.fieldZone.GetCount() <= 0)
            return false;
        bool hasDemon = false;
        player.recruitField.ForEachAlive([&hasDemon](const MinionData& data) {
            if (data.value().HasRace(Race::DEMON)) hasDemon = true;
        });
        return hasDemon;
    }
    if (definition->effect == ActivateEffect::DISCOVER_TAVERN_SPELL) {
        if (player.hand.IsFull() || targetIdx >= 0) return false;
        return std::any_of(Cards::GetAllCards().begin(), Cards::GetAllCards().end(),
            [](const Card& card) {
                return card.isBattlegroundsPoolSpell && card.normalDbfID == 0 &&
                    FindTavernSpellBehavior(card.id).effect != TavernSpellEffect::NONE;
            });
    }
    return targetIdx < 0;
}

int Minion::TriggerAvenge(Player& player)
{
    const auto& definition = m_card.power.GetAvenge();
    if (!definition || definition->threshold <= 0 || IsDestroyed()) return 0;
    ++m_avengeDeaths;
    int activations = 0;
    while (m_avengeDeaths >= definition->threshold)
    {
        m_avengeDeaths -= definition->threshold;
        ++activations;
        if (definition->effect == AvengeEffect::BUFF_SELF)
        {
            SetAttack(GetAttack() + definition->attack);
            SetHealth(GetHealth() + definition->health);
        }
        else if (definition->effect == AvengeEffect::BUFF_RACE)
        {
            // Avenge resolves during combat.  Use the active field so the
            // temporary Bird Buddy-style race buff affects combat copies;
            // permanent effects are committed to recruitField below.
            player.GetField().ForEachAlive([&](MinionData& data) {
                if (data.value().HasRace(definition->race))
                {
                    data.value().SetAttack(data.value().GetAttack() + definition->attack);
                    data.value().SetHealth(data.value().GetHealth() + definition->health);
                }
            });
        }
        else if (definition->effect == AvengeEffect::ADD_CARD && !definition->cardID.empty())
        {
            const Card card = Cards::FindCardByID(definition->cardID);
            for (int i = 0; !card.id.empty() && i < definition->cardCount && !player.hand.IsFull(); ++i)
            {
                Minion generated{ card };
                player.hand.Add(CardData{ std::move(generated) });
            }
        }
        else if (definition->effect == AvengeEffect::ADD_RANDOM_UNDEAD)
        {
            std::map<std::string, int> before;
            player.hand.ForEach([&before](const std::optional<CardData>& card) { if (card.has_value() && std::holds_alternative<Minion>(*card) && std::get<Minion>(*card).HasRace(Race::UNDEAD)) ++before[std::string(std::get<Minion>(*card).GetCardID())]; });
            SimpleTasks::RandomCardToHandTask task{Race::UNDEAD, 0, definition->cardCount};
            task.Run(player, *this);
            std::map<std::string, int> after;
            for (int i = 0; i < player.hand.GetCount(); ++i)
                if (std::holds_alternative<Minion>(player.hand[i]) && std::get<Minion>(player.hand[i]).HasRace(Race::UNDEAD))
                    ++after[std::string(std::get<Minion>(player.hand[i]).GetCardID())];
            for (const auto& [id, count] : after) for (int n = 0; n < count - before[id]; ++n) player.season14.TrackCombatAvengeCard(id);
        }
        else if (definition->effect == AvengeEffect::ADD_RANDOM_PROTOSS)
        {
            // Mothership's golden payload is represented by cardCount (1/2),
            // preserving the exact normal/golden Avenge fan-out and the
            // canonical Warp Gate Protoss eligibility filter.
            SimpleTasks::ProtossBehaviorTask::AddProtossToHand(
                player, definition->cardCount);
        }
        else if (definition->effect == AvengeEffect::PROGRESSIVE_END_TURN)
        {
            player.season14.progressiveAvengeAttack += definition->attack;
            player.season14.progressiveAvengeHealth += definition->health;
        }
        else if (definition->effect == AvengeEffect::PLAY_BLOOD_GEMS_RACE)
        {
            // ``attack`` stores the number of Blood Gems to play; unlike a
            // stat buff this must flow through Player's authoritative gem
            // resolver so race bonuses and persistent gem state apply.
            const bool portraitAllMinions =
                player.HasActivePortrait(PortraitEffect::BRISTLEBACH_ALL_MINIONS) &&
                (GetCardID() == "BG26_157" || GetCardID() == "BG26_157_G");
            player.GetField().ForEachAlive([&](MinionData& data) {
                if (!portraitAllMinions)
                {
                    if (!data.value().HasRace(definition->race)) return;
                }
                for (int i = 0; i < definition->attack; ++i)
                    player.ApplyBloodGemTo(data.value());
            });
        }
    }
    if (definition->permanent && activations > 0)
    {
        player.recruitField.ForEachAlive([this, &definition, activations](MinionData& data) {
            Minion& target = data.value();
            const bool same = GetIndex() >= 0 && target.GetIndex() == GetIndex();
            if ((definition->effect == AvengeEffect::BUFF_SELF && same) ||
                (definition->effect == AvengeEffect::BUFF_RACE && target.HasRace(definition->race)))
            {
                target.SetAttack(target.GetAttack() + definition->attack * activations);
                target.SetHealth(target.GetHealth() + definition->health * activations);
            }
        });
    }
    return activations;
}

void Minion::ResetAvengeProgress() { m_avengeDeaths = 0; }
const AvengeDefinition* Minion::GetAvengeDefinition() const
{
    const auto& definition = m_card.power.GetAvenge();
    return definition ? &*definition : nullptr;
}

bool Minion::Activate(Player& player, int targetIdx)
{
    if (!CanActivate(player, targetIdx))
    {
        return false;
    }
    const auto definition = *m_card.power.GetActivate();
    player.remainCoin -= definition.cost;
    player.RecordGoldSpent(definition.cost);
    if (definition.effect == ActivateEffect::BUFF_TARGET)
    {
        auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
        target.SetAttack(target.GetAttack() + definition.attack);
        target.SetHealth(target.GetHealth() + definition.health);
    }
    else if (definition.effect == ActivateEffect::SET_TARGET_STATS)
    {
        auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
        target.SetAttack(definition.attack);
        target.SetHealth(definition.health);
    }
    else if (definition.effect == ActivateEffect::GAIN_GOLD)
    {
        if (definition.nextTurn)
            player.season14.AddNextTurnGold(definition.amount);
        else
            player.remainCoin += definition.amount;
    }
    else if (definition.effect == ActivateEffect::ADD_CARD)
    {
        SimpleTasks::AddCardTask task{ definition.cardID, definition.amount };
        task.Run(player, *this);
    }
    else if (definition.effect == ActivateEffect::RANDOM_CARD)
    {
        SimpleTasks::RandomCardToHandTask task{definition.race, 0, definition.amount};
        task.Run(player, *this);
    }
    else if (definition.effect == ActivateEffect::RANDOM_CHROMADRAKE)
    {
        SimpleTasks::RandomChromadrakeToHandTask task{definition.amount};
        task.Run(player, *this);
    }
    else if (definition.effect == ActivateEffect::DISCOVER_TAVERN_SPELL)
    {
        player.BeginTavernSpellDiscover(
            definition.amount, static_cast<std::uint64_t>(GetIndex()),
            GetDbfID());
    }
    else if (definition.effect == ActivateEffect::TAVERN_STATS_RANDOM_KEYWORD)
    {
        // Deft Deserter rolls one keyword per activation, then applies that
        // same keyword to every minion currently in the Tavern.  Mutate the
        // Tavern field directly: these are shop instances, not the player's
        // recruit or combat field, and the +stats persist for this game via
        // the normal shop entity lifecycle.
        player.tavern.fieldZone.ForEachAlive([&](MinionData& data) {
            auto& tavernMinion = data.value();
            // The random choice is per eligible Tavern minion: the effect
            // targets all minions, while each target independently receives
            // one of the three listed keywords.
            const auto keyword = Random::get<int>(0, 2);
            tavernMinion.SetAttack(tavernMinion.GetAttack() + definition.attack);
            tavernMinion.SetHealth(tavernMinion.GetHealth() + definition.health);
            if (keyword == 0)
                tavernMinion.SetTaunt(true);
            else if (keyword == 1)
                tavernMinion.SetGameTag(GameTag::DIVINE_SHIELD, 1);
            else
                tavernMinion.SetGameTag(GameTag::WINDFURY, 1);
        });
    }
    else if (definition.effect == ActivateEffect::TRIGGER_RALLY)
    {
        auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
        // Golden Sky-hatch explicitly triggers the selected Rally twice.
        for (int i = 0; i < std::max(1, definition.amount); ++i)
            target.ActivateRally(player, *this, target);
    }
    else if (definition.effect == ActivateEffect::TRIGGER_BATTLECRY)
    {
        auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
        for (int i = 0; i < std::max(1, definition.amount); ++i)
        {
            target.ActivateTask(PowerType::POWER, player);
            // A targeted Battlecry/Discover can leave a public modal pending.
            // Do not invoke the golden copy a second time and overwrite the
            // first offering; the pending choice must be committed first.
            if (player.season14.pendingDecision != Season14Decision::NONE)
                break;
        }
    }
    else if (definition.effect == ActivateEffect::APPLY_REBORN)
    {
        auto& target = player.recruitField[static_cast<std::size_t>(targetIdx)];
        target.ApplyTemporaryKeyword(GameTag::REBORN);
    }
    else if (definition.effect == ActivateEffect::TAKE_HIGHEST_TAVERN)
    {
        for (int i = 0; i < std::max(1, definition.amount); ++i)
        {
            if (player.hand.IsFull()) break;
            int best = -1;
            player.tavern.fieldZone.ForEachAlive([&best](MinionData& data) {
                if (best < 0 || data.value().GetAttack() > best)
                    best = data.value().GetAttack();
            });
            if (best < 0) break;
            int slot = -1;
            int attack = 0;
            int health = 0;
            for (int j = 0; j < player.tavern.fieldZone.GetCount(); ++j)
            {
                auto& candidate = player.tavern.fieldZone[static_cast<std::size_t>(j)];
                if (!candidate.IsDestroyed() && candidate.GetAttack() == best)
                {
                    slot = j;
                    attack = candidate.GetAttack();
                    health = candidate.GetHealth();
                    break;
                }
            }
            if (slot < 0 || !player.TakeTavernMinionToHand(
                                  static_cast<std::size_t>(slot), attack, health))
                break;
        }
    }
    else if (definition.effect == ActivateEffect::DEVOUR_TAVERN_DEMONS)
    {
        player.DevourRandomTavernForDemons(std::max(1, definition.amount));
    }
    else if (definition.effect == ActivateEffect::DESTROY_UNDEAD_BUFF_SELF)
    {
        SimpleTasks::DestroyUndeadBuffSelfTask{definition.attack, definition.health}.Run(player, *this);
    }
    else if (definition.effect == ActivateEffect::ACTIVATE_FISHBAIT)
    {
        SimpleTasks::ActivateFishbaitTask{definition.cardID, definition.attack}
            .RunAt(player, *this, static_cast<std::size_t>(targetIdx));
    }
    else if (definition.effect == ActivateEffect::ACTIVATE_RANDOM_TAVERN_SPELLS)
    {
        SimpleTasks::ActivateRandomTavernSpellsTask{definition.amount}.Run(player, *this);
    }
    else if (definition.effect == ActivateEffect::ARM_MAGNETIZATION)
    {
        ArmMagnetization();
    }
    else if (definition.effect == ActivateEffect::GAIN_NEXT_BOUGHT_STATS)
    {
        player.ArmNextBoughtStats(GetIndex(), definition.amount);
    }
    --m_activateUses;
    return true;
}

void Minion::ResetActivateUses()
{
    m_activateUses = m_card.power.GetActivate().has_value() ? 1 : 0;
    m_magnetizationArmed = false;
    m_bloodGemCountThisTurn = 0;
    m_buyTriggerUses = 0;
}

bool Minion::CanUseBuyTrigger(int limit) const
{
    return limit <= 0 || m_buyTriggerUses < limit;
}

void Minion::ConsumeBuyTrigger()
{
    ++m_buyTriggerUses;
}

void Minion::ResetBuyTriggerUses()
{
    m_buyTriggerUses = 0;
}

std::vector<TaskType> Minion::GetTasks(PowerType type) const
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
