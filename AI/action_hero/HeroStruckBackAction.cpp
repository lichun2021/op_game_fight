//
//  HeroStruckBackAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroStruckBackAction.h"
#include "HeroActionController.h"
#include "HeroStandupAction.h"
#include "HeroAttackAction.h"

#include "cocos2d.h"
namespace OnePiece {
	void HeroStruckBackAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.localTick < aiCharacter.startTick + aiCharacter.totalTick)
		{
            aiCharacter.setX(aiContext.getAdjustedPosXWithinRange(aiCharacter.getX() + HeroActionController::getFlySpeedX(aiCharacter.actionAtt)));
		}
		else
		{
			HeroStandupAction::Enter(aiCharacter, aiContext, 4, STANDUP_ACTION, aiCharacter);
		}
	}

	void HeroStruckBackAction::Enter(CharacterData& aiCharacter, AIContext& aiContext, unsigned short tickLength, short speed)
	{
		if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
		{
            if (aiCharacter.hasTag("attacknobrokenwhenenter"))
                return;
            if (HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext))
                return;
			HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
			//CCLOG("DDDDDDDD:%d--%d--%d--%d", aiCharacter.myID, aiCharacter.attributes[0], aiCharacter.actionAtt[1], aiCharacter.localTick);
		}

        HeroActionController::exitAction(aiCharacter, aiContext);
		HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = STRUCK_BACKWORD_ACTION;
		aiCharacter.startTick=aiCharacter.localTick+1;
		aiCharacter.totalTick = tickLength;
		HeroActionController::setFlySpeedX(aiCharacter.actionAtt, speed * aiCharacter.staticData->damping);

        HeroActionController::didEnterAction(aiCharacter, aiContext);
	}

    void HeroStruckBackAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        aiCharacter.eraseTag("hurtresultstack");
    }
}

