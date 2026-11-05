//
//  HeroCelebrateAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroCelebrateAction.h"
#include "HeroIdleAction.h"
#include "HeroAttackAction.h"

namespace OnePiece {

	void HeroCelebrateAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.isFrozen) return;

		if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick) {
			HeroIdleAction::Enter(aiCharacter, aiContext, 5, IDLE_ACTION, aiCharacter);
		}
	}

    void HeroCelebrateAction::Enter(CharacterData& aiCharacter, AIContext& aiContext, unsigned short tickLength, ActionType actionType)
    {
        if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
        {
            HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext);
            HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
        }

        HeroActionController::exitAction(aiCharacter, aiContext);
        HeroActionController::cleanup(aiCharacter, aiContext, true);
        aiCharacter.currentAction = actionType;
        aiCharacter.startTick = aiCharacter.localTick + 1;
        aiCharacter.totalTick = tickLength;
        aiCharacter.actionCauserMyID = aiCharacter.actionCauserMyID;

        HeroActionController::didEnterAction(aiCharacter, aiContext);
    }

    void HeroCelebrateAction::Exit(CharacterData& aiCharacter, AIContext& aiContext)
    {
        aiCharacter.eraseTag("hurtresultstack");
    }
}

