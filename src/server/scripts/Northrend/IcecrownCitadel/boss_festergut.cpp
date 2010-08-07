/*
* Copyright (C) 2009 - 2010 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

// Scripted by Lavi & Andu - WoW-Romania Team http://www.wow-romania.ro (if you use this script, do not remove this seal, no matter what other modification you apply to script).

#include "ScriptPCH.h"
#include "icecrown_citadel.h"

enum Yells
{
    SAY_STINKI_DIES         =    -1666004,
    SAY_AGGRO               =    -1666005,
    SAY_GAS_CLOUD           =    -1666006,
    SAY_GAS_SPORES          =    -1666007,
    SAY_PUNGENT_BLIGHT_1    =    -1666008,
    SAY_PUNGENT_BLIGHT_2    =    -1666009,
    SAY_KILL_1              =    -1666010,
    SAY_KILL_2              =    -1666011,
    SAY_BERSERK             =    -1666012,
    SAY_DEATH_1             =    -1666013,
    SAY_PUTRICIDE_DEATH     =    -1666014,
};

enum Spells
{
    SPELL_PUNGENT_BLIGHT       =    69195,
    SPELL_GASTRIC_EXPLOSION    =    72227,
    SPELL_INHALE_BLIGHT        =    69165,
    SPELL_VILE_GAS             =    72272,
    SPELL_GASTRIC_BLOAT        =    72219,
    SPELL_GAS_VISUAL_SMAL      =    69154,
    SPELL_GAS_VISUAL_MIDDEL    =    69152,
    SPELL_GAS_VISUAL_BIG       =    69126,
    SPELL_GAS_SPORES           =    69278,
    SPELL_BERSERK              =    47008,
    SPELL_INOCULATED           =    72103,
    SPELL_BLIGHTED_SPORES      =    69290,
    SPELL_MORTAL_WOUND         =    71127,
    SPELL_DECIMATE             =    71123,
    SPELL_PLAGUE_STENCH        =    71161,
};

#define ACHIEV_INOCULATE       = RAID_MODE (4577, 4615)

#define EMOTE_GAS_SPORE "Festergut farts."
#define EMOTE_Pungent_Blight "Festergut vomits"

struct boss_festergutAI : public ScriptedAI
{
    boss_festergutAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = pCreature->GetInstanceData();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiPungentBlightTimer;
    uint32 m_uiGastricExplosionTimer;
    uint32 m_uiInhaleBlightTimer;
    uint32 m_uiGasSporesTimer;
    uint32 m_uiVileGasTimer;
    uint32 m_uiGastricBloatTimer;
    uint32 m_uiBerserkTimer;
    uint32 m_uiGastricBoom;
    uint64 uiPutricide;

    bool Achievements;

    void Reset()
    {
        m_uiPungentBlightTimer = 120000;
        m_uiInhaleBlightTimer  = 33000;
        m_uiVileGasTimer = 30000;
        m_uiGasSporesTimer = 21000;
        m_uiGastricBloatTimer = 15000;
        m_uiBerserkTimer = 300000;
        m_uiGastricBoom = 20000;
        uiPutricide = 0;

        Achievements = false;

        if (m_pInstance)
            m_pInstance->SetData(DATA_FESTERGURT_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO, me);

        if (m_pInstance)
            m_pInstance->SetData(DATA_FESTERGURT_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH_1, me);
        uiPutricide = (m_pInstance ? m_pInstance->GetData64(DATA_PROFESSOR_PUTRICIDE) : 0);
        if (Creature *pPutricide = me->GetCreature(*me, uiPutricide))
        DoScriptText(SAY_PUTRICIDE_DEATH, pPutricide);
        me->PlayDirectSound(17124);

        if (m_pInstance)
            m_pInstance->SetData(DATA_FESTERGURT_EVENT, DONE);
    }

    void JustReachedHome()
    {
        if(m_pInstance)
            m_pInstance->SetData(DATA_FESTERGURT_EVENT, FAIL);
    }

    void KilledUnit(Unit *victim)
    {
        switch(urand(0, 1))
        {
        case 0:
            DoScriptText(SAY_KILL_1, me);
            break;
        case 1:
            DoScriptText(SAY_KILL_2, me);
            break;
        }

        switch(0)
        {
        case 0:
            if (victim->HasAura(72103))
            {
                if(!Achievements)
                m_pInstance->DoCompleteAchievement(ACHIEV_INOCULATE);
                Achievements = true;
            }
            break;
        }
     }


    void SpellHitTarget(Unit *pTarget,const SpellEntry* spell)
    {
        switch(spell->Id)
        {
        case SPELL_GAS_SPORES:
            HandleTouchedSpells(pTarget, SPELL_BLIGHTED_SPORES);
            break;
        }
    }

    void HandleTouchedSpells(Unit *pTarget, uint32 TouchedType)
    {
        switch(TouchedType)
        {
        case SPELL_GAS_SPORES:
            if (pTarget && pTarget->HasAura(SPELL_BLIGHTED_SPORES))
            {
                pTarget->CastSpell(pTarget, SPELL_INOCULATED, true);
            }
            else
                pTarget->CastSpell(pTarget, SPELL_INOCULATED, true);
            break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        if (m_uiGastricBloatTimer < uiDiff)
        {
            Unit* pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);
            DoCast(pTarget, SPELL_GASTRIC_BLOAT);
            m_uiGastricBloatTimer = 15000;
        } else m_uiGastricBloatTimer -= uiDiff;

	if (m_uiGastricBoom < uiDiff)
	{
		Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true);
		if(pTarget && pTarget->HasAura(SPELL_GASTRIC_BLOAT))
			if (pTarget->GetAura(SPELL_GASTRIC_BLOAT, 0)->GetStackAmount() >= 10)
			{
				DoCast(pTarget, SPELL_GASTRIC_EXPLOSION);
				m_uiGastricBoom = 5000;
			} else m_uiGastricBoom -= uiDiff;
	}

        if(m_uiInhaleBlightTimer < uiDiff)
        {
            if (me->HasAura(SPELL_PUNGENT_BLIGHT))
            {
                me->RemoveAurasDueToSpell(SPELL_PUNGENT_BLIGHT);
            }
            DoCast(me, SPELL_INHALE_BLIGHT);
            m_uiInhaleBlightTimer = 33000;
        } else m_uiInhaleBlightTimer -= uiDiff;

        if (m_uiVileGasTimer < uiDiff)
        {
            Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1);
            DoCast(pTarget, SPELL_VILE_GAS);
            m_uiVileGasTimer = 37000;
        } else m_uiVileGasTimer -= uiDiff;

        if (getDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC || getDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
        {
            if (m_uiGasSporesTimer < uiDiff)
            {
                Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0);
                if (pTarget && !pTarget->HasAura(SPELL_GAS_SPORES))
                    {
                        DoCast(pTarget, SPELL_GAS_SPORES);
                        me->PlayDirectSound(16911);
                        me->MonsterTextEmote(EMOTE_GAS_SPORE, 0, true);
                    }
                m_uiGasSporesTimer = 23000;
            } else m_uiGasSporesTimer -= uiDiff;
        }

        if (m_uiPungentBlightTimer < uiDiff)
        {
            me->MonsterTextEmote(EMOTE_Pungent_Blight, 0, true);
            DoScriptText(SAY_PUNGENT_BLIGHT_1, me);
            DoCastAOE(SPELL_PUNGENT_BLIGHT);
            m_uiPungentBlightTimer = 120000;
            m_uiInhaleBlightTimer = 33000;
            me->RemoveAllAuras();
        } else m_uiPungentBlightTimer -= uiDiff;

        if(m_uiBerserkTimer < uiDiff)
        {
            DoCast(me, SPELL_BERSERK);
            m_uiBerserkTimer = 300000;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_stinkyAI : public ScriptedAI
{
    npc_stinkyAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = pCreature->GetInstanceData();
    }
    ScriptedInstance* m_pInstance;
    uint32 m_uiMortalWoundTimer;
    uint32 m_uiDecimateTimer;
    uint32 m_uiPlagueStench;
    uint64 uiFestergut;
    void Reset()
    {
   m_uiMortalWoundTimer          = 1500;
   m_uiDecimateTimer             = 23000;
   m_uiPlagueStench              = 46000;
   uiFestergut = 0;

    }

    void EnterCombat(Unit* who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_uiMortalWoundTimer <= diff)
        {
            Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);
            DoCast(pTarget, SPELL_MORTAL_WOUND);
            m_uiMortalWoundTimer = 10000;
        } else m_uiMortalWoundTimer -= diff;

        if (m_uiDecimateTimer <= diff)
        {
            Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);
            DoCast(pTarget, SPELL_DECIMATE);
            m_uiDecimateTimer = 17800;
        } else m_uiDecimateTimer -= diff;

        if (m_uiPlagueStench<= diff)
        {
            DoCastAOE(SPELL_PLAGUE_STENCH);
            m_uiPlagueStench = 12000;
        } else m_uiPlagueStench -= diff;

        DoMeleeAttackIfReady();
    }
    void JustDied(Unit* who)
    {
        uiFestergut = (m_pInstance ? m_pInstance->GetData64(DATA_FESTERGURT) : 0);
        if (Creature *pFestergut = me->GetCreature(*me, uiFestergut))
        DoScriptText(SAY_STINKI_DIES, pFestergut);
        me->PlayDirectSound(16907);
    }
};

CreatureAI* GetAI_boss_festergut(Creature* pCreature)
{
    return new boss_festergutAI(pCreature);
}

CreatureAI* GetAI_npc_stinky(Creature* pCreature)
{
    return new npc_stinkyAI(pCreature);
}

void AddSC_boss_festergut()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_festergut";
    newscript->GetAI = &GetAI_boss_festergut;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stinky";
    newscript->GetAI = &GetAI_npc_stinky;
    newscript->RegisterSelf();
}
