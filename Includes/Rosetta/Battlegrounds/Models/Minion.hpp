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

    //! Returns whether this card has the Magnetic keyword.
    bool IsMagnetic() const;

    //! Returns whether this Magnetic card may attach to the target.
    bool CanMagnetizeTo(const Minion& target) const;

    //! Merges this Magnetic card into an existing friendly target. The
    //! attachment's stats, supported keywords, and deathrattle tasks are
    //! transferred; the attachment itself is not placed on the board.
    void MagnetizeOnto(Minion& target) const;

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
    void ApplyPersistentRaceStats(Race race, int attack, int health);

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

    //! Sets the value of health.
    //! \param val The value of health to set.
    void SetHealth(int val);

    //! Applies recruit-turn-only Spellcraft stats/keywords.
    void ApplyTemporaryStats(int attack, int health, bool taunt = false);
    void ApplyTemporaryKeyword(GameTag tag);
    void ExpireTemporaryEffects();

    //! Returns the flag that indicates whether it has deathrattle.
    //! \return The flag that indicates whether it has deathrattle.
    bool HasDeathrattle() const;
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

    //! Returns whether this minion has the Reborn keyword available.
    //! \return true when the minion can be revived once after dying.
    bool HasReborn() const;

    //! Clears Reborn after the minion has been revived.
    //! \param reborn Whether Reborn remains available.
    void SetReborn(bool reborn);

    //! Returns whether this minion has Windfury.
    //! \return true when the minion attacks twice in a combat turn.
    bool HasWindfury() const;

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

    //! Configures a persistent Dark Gift bonus applied whenever a card is played.
    void SetPlayCardStatBonus(int attack, int health);
    void ApplyPlayCardStatBonus();
    void SetEndTurnBattlecryTrigger(bool enabled);
    bool HasEndTurnBattlecryTrigger() const;
    bool HasBattlecry() const;
    void SetDeathrattleStatTransfer(int attack, int health);
    int DeathrattleAttackTransfer() const;
    int DeathrattleHealthTransfer() const;
    void SetDarkGiftCounter(int attack, int health, int kind,
                            int currentCount = 0);
    void ApplyDarkGiftCounterStep(int kind);

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

    //! Sets whether this Tavern entity is frozen.
    //! \param frozen The new frozen state.
    void SetFrozen(bool frozen);

    //! Takes damage from a certain other minion.
    //! \param source A minion to give damage.
    void TakeDamage(Minion& source);

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
    //! Gets a list of tasks according to the power type.
    //! \param type The type of power.
    //! \return A list of tasks according to the power type.
    std::vector<TaskType> GetTasks(PowerType type);

    Card m_card;
    int m_index = -1;
    int m_poolIdx = -1;

    ZoneType m_zoneType = ZoneType::INVALID;
    int m_zonePos = -1;
    int m_lastFieldPos = -1;

    int m_attack = 0;
    int m_health = 0;
    int m_frenzyUses = 0;
    int m_globalMinionAttack = 0;
    int m_futureLobsterAttack = 0;
    int m_futureLobsterHealth = 0;
    int m_futureBallerAttack = 0;
    int m_futureBallerHealth = 0;
    std::array<int, 40> m_persistentRaceAttack{};
    std::array<int, 40> m_persistentRaceHealth{};
    int m_buyTriggerUses = 0;
    int m_bloodGemCount = 0;
    int m_bloodGemCountThisTurn = 0;
    int m_bloodGemAttack = 0;
    int m_bloodGemHealth = 0;
    int m_avengeDeaths = 0;
    int m_playCardAttackBonus = 0;
    int m_playCardHealthBonus = 0;
    bool m_endTurnBattlecryTrigger = false;
    int m_deathrattleAttackTransfer = 0;
    int m_deathrattleHealthTransfer = 0;
    int m_darkGiftCounterAttack = 0;
    int m_darkGiftCounterHealth = 0;
    int m_darkGiftCounterKind = 0;

    bool m_hasDeathrattle = false;
    bool m_hasTaunt = false;
    bool m_hasDivineShield = false;
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
    bool m_hasVenomous = false;
    bool m_hasStealth = false;
    bool m_isFrozen = false;
    bool m_isDestroyed = false;
    bool m_handLocked = false;
    int m_activateUses = 1;
    int m_startCombatAttackMultiplier = 1;
    int m_startCombatHealthMultiplier = 1;
    bool m_startCombatStatsApplied = false;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_MINION_HPP
