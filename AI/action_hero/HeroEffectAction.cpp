//
//  HeroEffectAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "../AIContext.h"
#include "HeroEffectAction.h"
#include "HeroActionController.h"
#include "HeroOutofControlAction.h"
#include "HeroAttackAction.h"
#include "../HeroHurtcalculate.h"

#include "cocos2d.h"

namespace OnePiece {
	void HeroEffectAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		HeroHurtcalculate::checkAndExecuteTalent(aiContext, &aiCharacter, &aiCharacter, HeroHurtcalculate::TalentTrigger::IN_EFFECT_ACTION);
        aiCharacter.m_bEnterEffectAction = false;
		if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick) {
			EffectExec *ee = AIDataHelper::getEffectExec(HeroActionController::getEffectID(aiCharacter.actionAtt));
			if (ee && ee->resumeToOrig > 0)
			{
				aiCharacter.currentAction = (ActionType)HeroActionController::getOldAction(aiCharacter.actionAtt);
				aiCharacter.totalTick = HeroActionController::getOldTotalTick(aiCharacter.actionAtt);
				aiCharacter.startTick = aiCharacter.localTick - (aiCharacter.totalTick - HeroActionController::getOldTick(aiCharacter.actionAtt));
			}
			else
			{
				HeroOutofControlAction::Enter(aiCharacter, aiContext, 2, IDLE_ACTION, aiCharacter);
			}
            
            if (aiCharacter.currentAction != EFFECT_ACTION)
                HeroActionController::setEffectID(aiCharacter.actionAtt, 0);
		}
	}

    bool HeroEffectAction::ExtendDuration(CharacterData& aiCharacter, int effectID, int extendTick, bool canEffBeDebuff, CharacterData& aiAttacker, AIContext& aiContext)
    {
        if (aiCharacter.currentAction != EFFECT_ACTION)
            return false;

        HeroActionController::setEffectID(aiCharacter.actionAtt, effectID);
        aiCharacter.totalTick += extendTick;
        aiCharacter.actionCauserMyID = aiAttacker.myID;
        if (!canEffBeDebuff)
            aiCharacter.addTag("effnodebuff");
        
        AICastBySpecState::checkExecute(aiContext, &aiCharacter, aiCharacter.actionCauserMyID, aiCharacter.currentAction);
        return true;
    }

    bool HeroEffectAction::Enter(CharacterData&aiCharacter, AIContext& aiContext, int effectID, int tickLength, ActionType oldActionType, int oldTotalTick, int oldLeftTick, bool canEffBeDebuff, CharacterData& aiAttacker)
	{
        if (aiCharacter.currentAction == EFFECT_ACTION || aiCharacter.currentAction == CONTROLLED_EFFECT_ACTION)
        {
            return false;
        }

		if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
		{
            if (aiCharacter.hasTag("attacknobrokenwhenenter"))
                return false;
            if (HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext))
                return false;
			HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
			//CCLOG("DDDDDDDD:%d--%d--%d--%d", aiCharacter.myID, aiCharacter.attributes[0], aiCharacter.actionAtt[1], aiCharacter.localTick);
		}

        HeroActionController::exitAction(aiCharacter, aiContext);
		//HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = EFFECT_ACTION;
		aiCharacter.startTick = aiCharacter.localTick+1;
		aiCharacter.totalTick = tickLength;
        aiCharacter.actionCauserMyID = aiAttacker.myID;

		HeroActionController::setOldAction(aiCharacter.actionAtt, oldActionType);
		HeroActionController::setOldTotalTick(aiCharacter.actionAtt, oldTotalTick);
		HeroActionController::setOldTick(aiCharacter.actionAtt, oldLeftTick);
		HeroActionController::setEffectID(aiCharacter.actionAtt, effectID);
        
        aiCharacter.m_bEnterEffectAction = true;
        if (!canEffBeDebuff)
            aiCharacter.addTag("effnodebuff");

        HeroActionController::didEnterAction(aiCharacter, aiContext);
        return true;
	}

    void HeroEffectAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_STATE_VERTIGO, 0) > 0)
        {
            AIStateVertigoEffect::checkSaveRemainTick(aiContext, &aiCharacter);
        }
        //HeroActionController::setEffectID(aiCharacter.actionAtt, 0);
        aiCharacter.eraseTag("effnodebuff");
    }
}

