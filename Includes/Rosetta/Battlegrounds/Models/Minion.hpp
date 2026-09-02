// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_MINION_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MINION_HPP

#include <Rosetta/Battlegrounds/Cards/Card.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

#include <initializer_list>
#include <array>
#include <cstdint>
#include <utility>

namespace RosettaStone::Battlegrounds
{
//!
//! \brief Minion class.
//!
//! Minions are persistent creatures on the battlefield that will fight for
//! their hero. Minion cards can be recognized by their Attack (a number
//! displayed on a yellow sword, in the bottom left corner) and Health (a number
//! displayed on a red blood drop, in the bottom right corner).
//!
class Minion
{
 public:
    //! Typed lifecycle payload for recruit-turn enchantments.  These effects
    //! are removed by ExpireTemporaryEffects at the next recruit start.
    enum class TemporaryEnchantment : std::uint8_t
    {
        Stats,
        StatsAndTaunt,
        StatsAndWindfury,
        StatsAndReborn,
        DivineShield,
        Venomous,
        StatsAndStealth,
    };

    //! Identity used when a recruit-phase entity is copied into combat.
    //! Entity IDs are preferred; pool/zone identity keeps deterministic
    //! hand-built fixtures usable without aliasing different cards.
    bool IsSameInstance(const Minion& other) const noexcept;

    //! Default constructor.
    Minion() = default;

    //! Constructs Minion instance with given \p card and \p poolIdx.
    //! \param card A card that contains the minion data.
    //! \param poolIdx The index of minion pool.
    explicit Minion(Card card, int poolIdx = -1);

    //! Returns the value of index.
    //! \return The value of index.
    int GetIndex() const;

    //! Sets the value of index.
    //! \param index The value of index.
    void SetIndex(int index);

    //! Returns the value of pool index.
    //! \return The value of pool index.
    int GetPoolIndex() const;

    //! Returns the value of name.
    //! \return The value of name.
    std::string_view GetName() const;

    //! Returns the stable Hearthstone card ID.
    //! \return The stable card ID.
    std::string_view GetCardID() const;

    //! Returns the stable Hearthstone database ID.
    //! \return The database ID.
    int GetDbfID() const;

    //! Returns the value of game tag.
    //! \param tag The game tag of card.
    //! \return The value of game tag.
    int GetGameTag(GameTag tag) const;

    //! Sets the value of game tag.
    //! \param tag The game tag to set.
    //! \param value The value of game tag to set.
    void SetGameTag(GameTag tag, int value);

    //! Sets whether this minion has Taunt.  Tavern-spell effects use the
    //! same mutable keyword state as combat resolution rather than changing
    //! card metadata.
    void SetTaunt(bool taunt);

    //! Returns the value of race.
    //! \return The value of race.
    Race GetRace() const;

    //! Returns whether this minion belongs to the requested tribe.
    //! \param race The tribe to test.
    //! \return true when this minion has the requested tribe.
    bool HasRace(Race race) const;
    void SetAmalgamation(bool enabled = true) noexcept { m_amalgamation = enabled; }
    //! Tarecgosa's Blessing keeps combat bonus keywords and doubles explicit
    //! combat stat gains when the combat copy is reconciled.
    void SetTarecgosaBlessing(bool enabled = true) noexcept { m_tarecgosaBlessing = enabled; }
    bool HasTarecgosaBlessing() const noexcept { return m_tarecgosaBlessing; }
    void SetTimeTurning(bool enabled = true) noexcept { m_timeTurning = enabled; }
    bool HasTimeTurning() const noexcept { return m_timeTurning; }
    void SetSteadyGrowth(int attack, int health) noexcept
    {
        m_steadyGrowthAttack = attack;
        m_steadyGrowthHealth = health;
    }
    bool HasSteadyGrowth() const noexcept { return m_steadyGrowthAttack != 0 || m_steadyGrowthHealth != 0; }
    void ApplySteadyGrowth() noexcept
    {
        m_attack += m_steadyGrowthAttack;
        m_health += m_steadyGrowthHealth;
    }
    // Reapplying the same gift is idempotent and must not reset a partially
    // elapsed two-turn cadence.  A changed captured race is a new target
    // choice and intentionally starts a fresh cadence.
    void SetAffinity(Race race) noexcept
    {
        if (m_affinityRace == race) return;
        m_affinityRace = race;
        m_affinityTurns = 0;
    }
    bool HasAffinity() const noexcept { return m_affinityRace != Race::INVALID; }
    bool AdvanceAffinity() noexcept
    {
        if (!HasAffinity()) return false;
        if (++m_affinityTurns < 2) return false;
        m_affinityTurns = 0;
        return true;
    }
    Race AffinityRace() const noexcept { return m_affinityRace; }
    void SetPolarization(bool enabled = true) noexcept { m_polarization = enabled; }
    bool HasPolarization() const noexcept { return m_polarization; }

    //! Returns whether this card has the Magnetic keyword.
    bool IsMagnetic() const;
    void ArmMagnetization();
    bool ConsumeMagnetizationArm();

    //! Returns whether this Magnetic card may attach to the target.
    bool CanMagnetizeTo(const Minion& target) const;

    //! Merges this Magnetic card into an existing friendly target. The
    //! attachment's stats, supported keywords, and deathrattle tasks are
    //! transferred; the attachment itself is not placed on the board.
    void MagnetizeOnto(Minion& target) const;
    int GetMagnetizationCount() const noexcept { return m_magnetizationCount; }

    //! Returns the value of zone type.
    //! \return The value of zone type.
    ZoneType GetZoneType() const;

    //! Sets the value of zone type.
    //! \param type The value of zone type.
    void SetZoneType(ZoneType type);

    //! Returns the value of zone position.
    //! \return The value of zone position.
    int GetZonePosition() const;

    //! Sets the value of zone position.
    //! \param pos The value of zone position.
    void SetZonePosition(int pos);

    //! Returns the value of last field position.
    //! \return The value of last field position.
    int GetLastFieldPos() const;

    //! Sets the value of last field position.
    //! \param pos The value of last field position.
    void SetLastFieldPos(int pos);

    //! Returns the value of tier.
    //! \return The value of tier.
    int GetTier() const;

    //! Returns whether this is the golden form of a minion.
    //! \return true when the metadata identifies a premium/golden entity.
    bool IsGolden() const;

    //! Converts this instance to its premium entity while preserving its
    //! current mutable stats and zone identity.
    //! \return false when no linked premium entity is available.
    bool MakeGolden();

    //! Returns whether this instance can be converted to a supported premium
    //! entity without mutating it.
    bool CanMakeGolden() const;
    //! Replaces card identity while preserving this instance's zone, stats,
    //! callbacks, and mutable keyword state.
    bool TransformTo(Card replacement);

    //! Returns the value of attack.
    //! \return The value of attack.
    int GetAttack() const;

    //! Sets the value of attack.
    //! \param val The value of attack to set.
    void SetAttack(int val);

    //! Applies the player's ALL Will Burn! attack aura to this instance.
    //! The operation is idempotent so a card can cross Tavern, hand, board,
    //! and combat boundaries without receiving the aura twice.
    void ApplyGlobalMinionAttack(int attack);

    //! Returns the ALL Will Burn! attack already applied to this instance.
    int GetGlobalMinionAttack() const;

    //! Applies the player's cumulative future-Lobster aura exactly once to
    //! this instance.  Re-running fresh-instance hooks while a card moves
    //! between Tavern, hand, and board must not stack the same aura.
    void ApplyFutureLobsterStats(int attack, int health);
    void ApplyFutureBallerStats(int attack, int health);
    //! Applies the cumulative all-minion Trinket aura idempotently.
    void ApplyPersistentMinionStats(int attack, int health);
    void ApplyPersistentTierMinionStats(int tier, int attack, int health);
    void ApplyPersistentRaceStats(Race race, int attack, int health);

    //! Fire-forged Evoker's lifetime Tavern-spell improvement, copied with
    //! the entity into combat and consumed by its Start of Combat task.
    void IncrementStartCombatSpellImprovement() noexcept
    { ++m_startCombatSpellImprovement; }
    int StartCombatSpellImprovement() const noexcept
    { return m_startCombatSpellImprovement; }

    //! Applies one Blood Gem's resolved stats and records the permanent
    //! instance count used by observation/diagnostics.
    void ApplyBloodGem(int attack, int health);

    //! Returns the number of Blood Gems applied to this instance.
    int GetBloodGemCount() const;

    //! Returns the number of Blood Gems applied since the last recruit start.
    int GetBloodGemsThisTurn() const;
    std::pair<int, int> RemoveBloodGems();

    //! Returns the value of health.
    //! \return The value of health.
    int GetHealth() const;
    //! Maximum Health is independent of combat damage for max-health effects.
    int GetMaxHealth() const;

    //! Sets the value of health.
    //! \param val The value of health to set.
    void SetHealth(int val);

    //! Applies recruit-turn-only Spellcraft stats/keywords.
    void ApplyTemporaryStats(int attack, int health, bool taunt = false);
    //! Applies a combat-time stat change that is explicitly permanent.  The
    //! delta is carried by combat copies and committed to the matching
    //! recruit entity when the battle ends; ordinary SetAttack/SetHealth
    //! calls remain combat-only.
    void ApplyCombatPersistentStats(int attack, int health);
    //! Applies a combat-time keyword that is explicitly permanent.
    void ApplyCombatPersistentKeyword(GameTag tag);
    //! Commits only explicitly persistent combat deltas from a matching copy.
    void ReconcileCombatPersistentState(const Minion& combatCopy);
    void BeginPoetCombatSnapshot(bool eligible, int multiplier = 1) noexcept;
    bool IsPoetCombatEligible() const noexcept { return m_poetCombatEligible; }
    int PoetCombatAttack() const noexcept { return m_poetCombatAttack; }
    int PoetCombatHealth() const noexcept { return m_poetCombatHealth; }
    std::uint32_t PoetCombatKeywords() const noexcept { return m_poetCombatKeywords; }
    int PoetCombatMultiplier() const noexcept { return m_poetCombatMultiplier; }
    void ApplyTemporaryKeyword(GameTag tag);
    //! Apply one typed temporary enchantment payload.  Keeping the lifecycle
    //! choice in Minion prevents individual Tavern spells from duplicating
    //! expiry bookkeeping.
    void ApplyTemporaryEnchantment(TemporaryEnchantment kind, int attack = 0,
                                   int health = 0);
    //! Applies the cumulative Falling Sky Golem deathrattle aura exactly once
    //! per observed deathrattle.  The applied count is instance state so an
    //! existing minion can safely pass through fresh-modifier setup again.
    void ApplySkyGolemDeathrattleCount(int count);
    //! Applies the owning player's cumulative Eternal Knight death aura
    //! exactly once per newly observed friendly Knight death.
    void ApplyEternalKnightDeathCount(int count);
    void ExpireTemporaryEffects();
    //! Resets the per-recruit-turn Lava Lurker Spellcraft allowance.
    void ResetSpellcraftUses() noexcept;
    bool ConsumeSpellcraftUse() noexcept;
    int GetSpellcraftUsesRemaining() const noexcept { return m_spellcraftUsesRemaining; }
    void ResetZestyShakerUse() noexcept { m_zestyShakerUsed = false; }
    bool ConsumeZestyShakerUse() noexcept { if (m_zestyShakerUsed) return false; m_zestyShakerUsed = true; return true; }
    bool AdvanceFelboarSpellCounter() noexcept { return ++m_felboarSpellCounter % 3 == 0; }
    bool ConsumeKodoSummonUse() noexcept { if (m_kodoSummonUses >= 3) return false; ++m_kodoSummonUses; return true; }
    void SetPermanentSpellcraft(bool enabled = true) noexcept { m_permanentSpellcraft = enabled; }
    bool HasPermanentSpellcraft() const noexcept { return m_permanentSpellcraft; }
    bool IsLavaLurker() const noexcept;

    //! Returns the flag that indicates whether it has deathrattle.
    //! \return The flag that indicates whether it has deathrattle.
    bool HasDeathrattle() const;
    //! Returns the task list for a requested power phase.  The native bridge
    //! uses this read-only query to reconstruct exact legality.
    std::vector<TaskType> GetTasks(PowerType type) const;
    //! Copies the currently configured deathrattle tasks to another instance.
    //! Used by recursive combat deathrattle effects such as Leapfrogger.
    void CopyDeathrattleTo(Minion& target) const;
    //! Attaches a Dark Gift-owned task to this instance's persistent power.
    void AddDarkGiftRallyTask(TaskType&& task);
    void AddDarkGiftDeathrattleTask(TaskType&& task);

    //! Returns the flag that indicates whether it has taunt.
    //! \return The flag that indicates whether it has taunt.
    bool HasTaunt() const;

    //! Returns the flag that indicates whether it has divine shield.
    //! \return The flag that indicates whether it has divine shield.
    bool HasDivineShield() const;
    void SetDivineShieldHits(int hits);
    int DivineShieldHitsRemaining() const;

    //! Returns whether this minion has the Reborn keyword available.
    //! \return true when the minion can be revived once after dying.
    bool HasReborn() const;

    //! Clears Reborn after the minion has been revived.
    //! \param reborn Whether Reborn remains available.
    void SetReborn(bool reborn);

    //! Returns whether this minion has Windfury.
    //! \return true when the minion attacks twice in a combat turn.
    bool HasWindfury() const;
    bool HasMegaWindfury() const { return m_hasMegaWindfury; }

    //! Returns whether this minion has the Battlegrounds Venomous keyword.
    //! \return true when damage from this minion destroys its target.
    bool HasVenomous() const;

    //! Returns whether this minion has Stealth.
    bool HasStealth() const;

    //! Configures one-shot stat multipliers applied at combat start.
    void SetStartCombatStatMultipliers(int attackMultiplier,
                                       int healthMultiplier);

    //! Applies and consumes the combat-start stat multipliers.
    void ApplyStartCombatStatMultipliers();

    //! Configures a Dark Gift that triggers this minion's Deathrattles at
    //! the beginning of combat.
    void SetStartCombatDeathrattleTrigger(bool enabled);
    bool HasStartCombatDeathrattleTrigger() const;
    bool ConsumeStartCombatDeathrattleTrigger();
    //! Configures a Dark Gift that copies the immediate left minion's attack
    //! into this minion at combat start.
    void SetStartCombatLeftAttack(bool enabled);
    bool HasStartCombatLeftAttack() const;
    bool ConsumeStartCombatLeftAttack();
    void ApplyStartCombatLeftAttack(const Minion& left);
    void SetImmuneWhileAttacking(bool enabled);
    bool HasImmuneWhileAttacking() const;
    void SetAttacking(bool attacking);
    bool IsAttacking() const;

    //! Configures a persistent Dark Gift bonus applied whenever a card is played.
    void SetPlayCardStatBonus(int attack, int health);
    void ApplyPlayCardStatBonus();
    void SetAttackThresholdDivineShield(int threshold);
    void SetEndTurnBattlecryTrigger(bool enabled);
    void SetSpendGoldThresholdFired(bool fired) noexcept { m_spendGoldThresholdFired = fired; }
    bool SpendGoldThresholdFired() const noexcept { return m_spendGoldThresholdFired; }
    void SetSpendGoldThresholdCount(int count) noexcept { m_spendGoldThresholdCount = count; }
    int SpendGoldThresholdCount() const noexcept { return m_spendGoldThresholdCount; }
    void AddDamageDealt(int amount) noexcept { m_damageDealt += std::max(0, amount); }
    int DamageDealt() const noexcept { return m_damageDealt; }
    bool TreasureParrotRewarded() const noexcept { return m_treasureParrotRewarded; }
    void SetTreasureParrotRewarded(bool value) noexcept { m_treasureParrotRewarded = value; }
    void SetHeroDamageThresholdFired(bool fired) noexcept { m_heroDamageThresholdFired = fired; }
    bool HeroDamageThresholdFired() const noexcept { return m_heroDamageThresholdFired; }
    bool HasEndTurnBattlecryTrigger() const;
    void SetTaughtTavernSpell(std::string spellID) { m_taughtTavernSpell = std::move(spellID); }
    const std::string& TaughtTavernSpell() const noexcept { return m_taughtTavernSpell; }
    bool HasBattlecry() const;
    void SetDeathrattleStatTransfer(int attack, int health);
    //! Configures the transfer to affect every surviving friendly minion.
    //! The default transfer targets the first surviving recipient (Dark Gift).
    void SetDeathrattleStatTransferToAll(bool enabled);
    bool DeathrattleStatTransferToAll() const;
    void SetEarthElementalDeathrattle(bool enabled);
    bool HasEarthElementalDeathrattle() const;
    int DeathrattleAttackTransfer() const;
    int DeathrattleHealthTransfer() const;
    void SetDarkGiftCounter(int attack, int health, int kind,
                            int currentCount = 0);
    void ApplyDarkGiftCounterStep(int kind);
    //! Arms Incubation and advances its recruit-turn countdown.
    void SetIncubation(int turns = 2);
    void AdvanceIncubation();
    int IncubationTurnsRemaining() const;
    //! Number of completed recruit turns Patient Scout has waited.
    void AdvancePatientScout() noexcept { if (m_patientScoutTurns < 6) ++m_patientScoutTurns; }
    int PatientScoutTurns() const noexcept { return m_patientScoutTurns; }
    //! Arms Replication and advances its two-turn recruit countdown.
    void SetReplication(int turns = 2);
    bool AdvanceReplication();

    //! Returns the number of attacks this minion may make in one combat turn.
    //! \return One, two, or four for normal, Windfury, or Mega Windfury.
    int GetAttackCount() const;

    //! Revives this minion according to the Reborn keyword.
    //! The revived copy has one Health and cannot Reborn again.
    void ReviveWithReborn();

    //! Returns whether this Tavern entity is frozen.
    //! \return true if this entity is frozen, false otherwise.
    bool IsFrozen() const;
    bool IsHandLocked() const { return m_handLocked; }
    void SetHandLocked(bool locked) { m_handLocked = locked; }
    bool HasCombinedChooseOne() const { return m_combinedChooseOne; }
    void SetCombinedChooseOne(bool enabled) { m_combinedChooseOne = enabled; }
    bool DiesAtRecruitEnd() const { return m_diesAtRecruitEnd; }
    void SetDiesAtRecruitEnd(bool enabled) { m_diesAtRecruitEnd = enabled; }

    //! Sets whether this Tavern entity is frozen.
    //! \param frozen The new frozen state.
    void SetFrozen(bool frozen);

    //! Takes damage from a certain other minion.
    //! \param source A minion to give damage.
    void TakeDamage(Minion& source);
    void SetLastDamageSource(const Minion& source) noexcept;
    int LastDamageSourceIndex() const noexcept { return m_lastDamageSourceIndex; }
    const std::string& LastDamageSourceCardID() const noexcept { return m_lastDamageSourceCardID; }
    void DestroyImmediately() noexcept { m_isDestroyed = true; m_health = 0; }

    //! Takes damage to the minion.
    //! \param amount The amount of damage.
    void TakeDamage(int amount);
    void ResetFrenzyUses();

    //! Returns the flag that indicates whether it is destroyed.
    //! \return The flag that indicates whether it is destroyed.
    bool IsDestroyed() const;

    //! Gets a value indicating whether source entity is playable by card
    //! requirements. Static requirements are checked.
    //! \param player The owner of the minion.
    //! \return true if it is playable by card requirements, false otherwise.
    bool IsPlayableByCardReq(Player& player) const;

    //! Gets whether the current field has any valid play targets
    //! for this playable.
    //! \param player The owner of the minion.
    //! \return true if the current field has any valid play targets,
    //! false otherwise.
    bool HasAnyValidPlayTargets(Player& player) const;

    //! Determines whether the specified character is a valid target.
    //! \param player The owner of the minion.
    //! \param targetIdx The index of proposed target.
    //! \return true if the specified target is valid, false otherwise.
    bool IsValidPlayTarget(Player& player, int targetIdx);

    //! Checks the targeting type of a card.
    //! \param target The proposed target.
    //! \return true if the targeting type is valid, false otherwise.
    bool CheckTargetingType(Minion& target);

    //! Activates the trigger.
    //! \param type The type of trigger.
    //! \param source The source of trigger.
    void ActivateTrigger(TriggerType type, Minion& source);

    //! Activates this instance's hero-damage trigger.  The source is the
    //! observing minion itself so SELF filtering remains exact.
    void ActivateHeroDamageTrigger();

    //! Activates the task.
    //! \param type The type of power.
    //! \param player The owner of the minion.
    void ActivateTask(PowerType type, Player& player);

    //! Activates the task.
    //! \param type The type of power.
    //! \param player The owner of the minion.
    //! \param target The target.
    void ActivateTask(PowerType type, Player& player, Minion& target);

    //! Resolves this minion's Rally effect after a friendly attack is
    //! declared. The target is the selected opposing minion.
    void ActivateRally(Player& player, Minion& source, Minion& target);

    //! Returns whether this instance has a legal manual Activate available.
    bool CanActivate(const Player& player, int targetIdx = -1) const;
    //! Kelp Keeper cannot supply a second target while replaying a Battlecry.
    bool RequiresPlayTarget() const noexcept
    {
        return m_card.mustHaveToTargetToPlay;
    }
    int TriggerAvenge(Player& player);
    void ResetAvengeProgress();
    const AvengeDefinition* GetAvengeDefinition() const;

    //! Consumes gold and resolves this instance's manual Activate effect.
    bool Activate(Player& player, int targetIdx = -1);

    //! Re-arms a once-per-recruit-turn Activate.
    void ResetActivateUses();
    bool CanUseBuyTrigger(int limit) const;
    void ConsumeBuyTrigger();
    void ResetBuyTriggerUses();

    Trigger activatedTrigger;

    std::function<Player&()> getPlayerCallback;

 private:
    void NotifyPersistentAttackGain(int amount);

 private:
    Card m_card;
    int m_index = -1;
    int m_poolIdx = -1;

    ZoneType m_zoneType = ZoneType::INVALID;
    int m_zonePos = -1;
    int m_lastFieldPos = -1;

    int m_attack = 0;
    int m_health = 0;
    int m_maxHealth = 0;
    int m_frenzyUses = 0;
    int m_globalMinionAttack = 0;
    int m_futureLobsterAttack = 0;
    int m_futureLobsterHealth = 0;
    int m_futureBallerAttack = 0;
    int m_futureBallerHealth = 0;
    int m_persistentMinionAttack = 0;
    int m_persistentMinionHealth = 0;
    int m_persistentTierMinionAttack = 0;
    int m_persistentTierMinionHealth = 0;
    std::array<int, 40> m_persistentRaceAttack{};
    std::array<int, 40> m_persistentRaceHealth{};
    int m_startCombatSpellImprovement = 0;
    int m_buyTriggerUses = 0;
    int m_bloodGemCount = 0;
    int m_bloodGemCountThisTurn = 0;
    int m_bloodGemAttack = 0;
    int m_bloodGemHealth = 0;
    int m_combatPersistentAttack = 0;
    int m_combatPersistentHealth = 0;
    std::uint32_t m_combatPersistentKeywords = 0;
    int m_poetCombatAttack = 0;
    int m_poetCombatHealth = 0;
    std::uint32_t m_poetCombatKeywords = 0;
    bool m_poetCombatEligible = false;
    int m_poetCombatMultiplier = 1;
    bool m_tarecgosaBlessing = false;
    int m_steadyGrowthAttack = 0;
    int m_steadyGrowthHealth = 0;
    Race m_affinityRace = Race::INVALID;
    int m_affinityTurns = 0;
    bool m_polarization = false;
    int m_avengeDeaths = 0;
    int m_playCardAttackBonus = 0;
    int m_playCardHealthBonus = 0;
    int m_attackThresholdDivineShield = 0;
    bool m_attackThresholdTriggered = false;
    bool m_endTurnBattlecryTrigger = false;
    bool m_spendGoldThresholdFired = false;
    int m_spendGoldThresholdCount = 0;
    int m_damageDealt = 0;
    bool m_treasureParrotRewarded = false;
    std::string m_taughtTavernSpell;
    bool m_heroDamageThresholdFired = false;
    int m_deathrattleAttackTransfer = 0;
    int m_deathrattleHealthTransfer = 0;
    bool m_deathrattleStatTransferToAll = false;
    bool m_earthElementalDeathrattle = false;
    int m_skyGolemDeathrattleCount = 0;
    int m_eternalKnightDeathCountApplied = 0;
    int m_darkGiftCounterAttack = 0;
    int m_darkGiftCounterHealth = 0;
    int m_darkGiftCounterKind = 0;
    int m_incubationTurnsRemaining = 0;
    int m_replicationTurnsRemaining = 0;
    int m_patientScoutTurns = 0;

    bool m_hasDeathrattle = false;
    bool m_hasTaunt = false;
    bool m_hasDivineShield = false;
    int m_divineShieldHitsRemaining = 0;
    bool m_hasReborn = false;
    bool m_hasWindfury = false;
    bool m_hasMegaWindfury = false;
    int m_temporaryAttack = 0;
    int m_temporaryHealth = 0;
    bool m_temporaryTaunt = false;
    bool m_temporaryDivineShield = false;
    bool m_temporaryReborn = false;
    bool m_temporaryWindfury = false;
    bool m_temporaryMegaWindfury = false;
    bool m_temporaryVenomous = false;
    bool m_temporaryStealth = false;
    int m_spellcraftUsesRemaining = 0;
    bool m_permanentSpellcraft = false;
    bool m_zestyShakerUsed = false;
    int m_felboarSpellCounter = 0;
    int m_kodoSummonUses = 0;
    bool m_hasVenomous = false;
    bool m_hasStealth = false;
    bool m_isFrozen = false;
    bool m_isDestroyed = false;
    bool m_magnetizationArmed = false;
    int m_magnetizationCount = 0;
    bool m_handLocked = false;
    bool m_combinedChooseOne = false;
    bool m_diesAtRecruitEnd = false;
    int m_activateUses = 1;
    int m_startCombatAttackMultiplier = 1;
    int m_startCombatHealthMultiplier = 1;
    bool m_startCombatStatsApplied = false;
    std::uint8_t m_startCombatDeathrattleTriggers = 0;
    std::uint8_t m_startCombatLeftAttackTriggers = 0;
    bool m_immuneWhileAttacking = false;
    bool m_isAttacking = false;
    bool m_amalgamation = false;
    bool m_timeTurning = false;
    int m_lastDamageSourceIndex = -1;
    std::string m_lastDamageSourceCardID;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_MINION_HPP
