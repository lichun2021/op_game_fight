#include "HeroTurnAction.h"
#include "HeroAttackAction.h"

#include "cocos2d.h"
namespace OnePiece {
	void HeroTurnAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
		if (aiCharacter.currentAction != TURN_ACTION) return;
		HeroActionController::actionSelector(aiCharacter, aiContext);
	}

	void HeroTurnAction::Enter(CharacterData&aiCharacter, AIContext& aiContext)
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
		aiCharacter.direction = -1 * aiCharacter.direction;
		aiCharacter.currentAction = TURN_ACTION;
        //aiCharacter.lastDirection = aiCharacter.direction; // MARK:该数值现在仅在移动执行中使用，强制转向时始终保持两个数值一致，待优化

        HeroActionController::didEnterAction(aiCharacter, aiContext);
	}
}