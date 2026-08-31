// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/Battlegrounds/CardSets/BattlegroundsCardsGen.hpp>
#include <Rosetta/Battlegrounds/CardSets/DarkGiftBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/BloodGemBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch2.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch3.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch4.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch5.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch6.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch7.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch8.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch9.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch10.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch11.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch12.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsSimpleBatch.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch16.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch13.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch15.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch17.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch19.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch20.hpp>
#include <Rosetta/Battlegrounds/CardSets/SpellcraftMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch21.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch22.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch23.hpp>
#include <Rosetta/Battlegrounds/CardSets/MagneticMinionBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch24.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch28.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch30.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch32.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch33.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch34.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch35.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch36.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch37.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch39.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch40.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch41.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch42.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch43.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch44.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch45.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch46.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch47.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch48.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch50.hpp>
#include <Rosetta/Battlegrounds/CardSets/GeneratedBehaviorMappings.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch31.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch26.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch25.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch29.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatchBaller.hpp>
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch14.hpp>
#include <Rosetta/Battlegrounds/CardSets/ActivateBehaviors.hpp>
#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>
#include <Rosetta/Battlegrounds/Cards/CardDefs.hpp>

namespace RosettaStone::Battlegrounds
{
std::map<std::string, CardDef> CardDefs::m_data;

CardDefs::CardDefs()
{
    BattlegroundsCardsGen::AddAll(m_data);
    DarkGiftBehaviors::AddAll(m_data);
    BloodGemBehaviors::AddAll(m_data);
    ModernMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch2::AddAll(m_data);
    ModernMinionBehaviorsBatch3::AddAll(m_data);
    ModernMinionBehaviorsBatch4::AddAll(m_data);
    ModernMinionBehaviorsBatch5::AddAll(m_data);
    ModernMinionBehaviorsBatch6::AddAll(m_data);
    ModernMinionBehaviorsBatch7::AddAll(m_data);
    ModernMinionBehaviorsBatch8::AddAll(m_data);
    ModernMinionBehaviorsBatch9::AddAll(m_data);
    ModernMinionBehaviorsBatch10::AddAll(m_data);
    ModernMinionBehaviorsBatch11::AddAll(m_data);
    ModernMinionBehaviorsBatch12::AddAll(m_data);
    ModernMinionBehaviorsSimpleBatch::AddAll(m_data);
    ModernMinionBehaviorsBatch16::AddAll(m_data);
    ModernMinionBehaviorsBatch13::AddAll(m_data);
    ModernMinionBehaviorsBatch15::AddAll(m_data);
    ModernMinionBehaviorsBatch17::AddAll(m_data);
    ModernMinionBehaviorsBatch19::AddAll(m_data);
    ModernMinionBehaviorsBatch20::AddAll(m_data);
    SpellcraftMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch21::AddAll(m_data);
    ModernMinionBehaviorsBatch22::AddAll(m_data);
    ModernMinionBehaviorsBatch23::AddAll(m_data);
    MagneticMinionBehaviors::AddAll(m_data);
    ModernMinionBehaviorsBatch24::AddAll(m_data);
    ModernMinionBehaviorsBatch28::AddAll(m_data);
    ModernMinionBehaviorsBatch30::AddAll(m_data);
    ModernMinionBehaviorsBatch32::AddAll(m_data);
    ModernMinionBehaviorsBatch33::AddAll(m_data);
    ModernMinionBehaviorsBatch34::AddAll(m_data);
    ModernMinionBehaviorsBatch35::AddAll(m_data);
    ModernMinionBehaviorsBatch36::AddAll(m_data);
    ModernMinionBehaviorsBatch37::AddAll(m_data);
    ModernMinionBehaviorsBatch39::AddAll(m_data);
    ModernMinionBehaviorsBatch40::AddAll(m_data);
    ModernMinionBehaviorsBatch41::AddAll(m_data);
    ModernMinionBehaviorsBatch42::AddAll(m_data);
    ModernMinionBehaviorsBatch43::AddAll(m_data);
    ModernMinionBehaviorsBatch44::AddAll(m_data);
    ModernMinionBehaviorsBatch45::AddAll(m_data);
    ModernMinionBehaviorsBatch46::AddAll(m_data);
    ModernMinionBehaviorsBatch47::AddAll(m_data);
    ModernMinionBehaviorsBatch48::AddAll(m_data);
    ModernMinionBehaviorsBatch50::AddAll(m_data);
    GeneratedBehaviorMappings::AddAll(m_data);
    ModernMinionBehaviorsBatch31::AddAll(m_data);
    ModernMinionBehaviorsBatch26::AddAll(m_data);
    ModernMinionBehaviorsBatch25::AddAll(m_data);
    ModernMinionBehaviorsBatch29::AddAll(m_data);
    ModernMinionBehaviorsBatchBaller::AddAll(m_data);
    ModernMinionBehaviorsBatch14::AddAll(m_data);
    ActivateBehaviors::AddAll(m_data);
    TrinketBehaviors::AddAll(m_data);
}

CardDefs::~CardDefs()
{
    m_data.clear();
}

CardDefs& CardDefs::GetInstance()
{
    static CardDefs instance;
    return instance;
}

CardDef CardDefs::FindCardDefByID(const std::string_view& id)
{
    for (auto& data : m_data)
    {
        if (data.first == id)
        {
            return data.second;
        }
    }

    return CardDef();
}

bool CardDefs::HasDefinition(const std::string_view& id)
{
    (void)GetInstance();
    return m_data.contains(std::string(id));
}
}  // namespace RosettaStone::Battlegrounds
