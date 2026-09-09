#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion;
namespace SimpleTasks {
//! Exact player-owned hooks for the hero-generated Protoss pool.
class ProtossBehaviorTask {
 public:
  enum class Effect : unsigned char { MOTHERSHIP_REWARD, IMMORTAL_DOUBLE,
                                      SENTRY_DISCOUNT, VOID_RAY_BONUS,
                                      COLOSSUS_DAMAGE, HIGH_TEMPLAR_DAMAGE,
                                      DARK_TEMPLAR_DESTROY, CARRIER_END_TURN,
                                      ARCHON_END_TURN };
  explicit ProtossBehaviorTask(Effect effect) : m_effect(effect) {}
  //! Add the requested number of eligible Protoss cards to the player's hand.
  //! This is shared by Mothership's normal/golden Avenge payload and the
  //! ordinary Warp Gate reward, keeping pool filtering in one owner.
  static void AddProtossToHand(Player&, int count);
  TaskStatus Run(Player&, Minion&);
  TaskStatus Run(Player&, Minion&, Minion&);
 private: Effect m_effect;
};
}}
