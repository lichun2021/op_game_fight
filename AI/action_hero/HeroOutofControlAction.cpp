//
//  HeroOutofControlAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroOutofControlAction.h"
#include "HeroAttackAction.h"
#include "../scriptbind/AIScriptBindActionController.hpp"

#include "cocos2d.h"
namespace OnePiece {
	void HeroOutofControlAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
        if (aiCharacter.target == nullptr || HeroActionController::isDeadAction(aiCharacter.target->currentAction))
		{
			if (aiCharacter.currentAction != ActionType::CONTROLLED_EFFECT_ACTION)
			{
				HeroActionController::actionSelector(aiCharacter, aiContext);
				return;
			}
		}

		if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick) {
			HeroActionController::actionSelector(aiCharacter, aiContext);
		}
	}

	bool HeroOutofControlAction::Enter(CharacterData&aiCharacter, AIContext& aiContext, unsigned short tickLength, ActionType actionType, CharacterData& aiAttacker)
	{
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
		HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = actionType;
		aiCharacter.startTick = aiCharacter.localTick+1;
		aiCharacter.totalTick = tickLength;
        aiCharacter.actionCauserMyID = aiAttacker.myID;

        HeroActionController::didEnterAction(aiCharacter, aiContext);
		return true;
    }
    
    void HeroOutofControlAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        luahelper_outofcontrolaction_on_exit(aiContext.getLuaState(), &aiCharacter);
    }

    void HeroOutofControlAction::ExitNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        aiCharacter.eraseTag("hurtresultstack");
    }
}
